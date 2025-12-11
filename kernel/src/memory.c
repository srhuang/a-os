#include "memory.h"
#include "stddef.h"
#include "string.h"
#include "print.h"
#include "math.h"
#include "list.h"

//=========================
// debugging
//=========================
#define DEBUG (0)
#define TRACE_STR(x) do {if(DEBUG) put_str(x);} while(0)
#define TRACE_INT(x) do {if(DEBUG) put_int(x);} while(0)

//=========================
// internal struct
//=========================
struct arena {
   struct mem_block_desc* p_mem_block;
   // If is_page_cnt is true, cnt represents the number of pages.
   // Otherwise, cnt represents the number of memory blocks.
   uint32_t cnt;
   bool is_page_cnt;
};

// for each memory block in arena
struct mem_block {
    struct list_elem free_elem;
};

//=========================
// global variable
//=========================
// for physical memory management
struct p_pool k_p_pool, u_p_pool;

// for kernel virtual address
struct v_pool k_v_pool;

// for memory block management
struct mem_block_desc   k_mem_block[MEM_BLOCK_CNT];

// lock for page table
static struct mutex mlock_pgtable;

//=========================
// internal functions
//=========================

static uint32_t* pde_ptr(void* vaddr)
{
    uint32_t vaddr_val = (uint32_t)vaddr;
    uint32_t* pde = (uint32_t*)((0xfffff000) + PDE_IDX(vaddr_val) * 4);
    return pde;
}

static uint32_t* pte_ptr(void* vaddr)
{
    uint32_t vaddr_val = (uint32_t)vaddr;
    uint32_t* pte = (uint32_t*)(0xffc00000 + \
        ((vaddr_val & 0xffc00000) >> 10) + \
        PTE_IDX(vaddr_val) * 4);
   return pte;
}

static uint32_t addr_v2p(void* vaddr)
{
    uint32_t* pte = pte_ptr(vaddr);
    uint32_t vaddr_val = (uint32_t)vaddr;
    return ((*pte & 0xfffff000) + (vaddr_val & 0x00000fff));
}

static void* vaddr_acquire(struct v_pool* vp, void* vaddr, int pg_cnt)
{
    void* vaddr_ret = NULL;
    uint32_t vaddr_val = (uint32_t)vaddr;
    int btmp_idx = -1;

    if (NULL == vaddr) {
        btmp_idx  = bitmap_acquire(&vp->vaddr_bitmap, pg_cnt);

        //* for debugging
        TRACE_STR("vaddr acquire:btmp_idx=0x");
        TRACE_INT(btmp_idx);
        TRACE_STR(", cnt=0x");
        TRACE_INT(pg_cnt);
        TRACE_STR("\n");
        //*/

        if (-1 == btmp_idx) {
            return NULL;
        }
        vaddr_ret = (void*)vp->vaddr_start + (btmp_idx * PG_SIZE);
    } else {
        btmp_idx = (vaddr_val - vp->vaddr_start) / PG_SIZE;

        // check and set each virtual address bitmap
        int cnt = 0;
        while (cnt < pg_cnt) {
            if (bitmap_check(&vp->vaddr_bitmap, btmp_idx + cnt)) {
                return NULL;
            }
            bitmap_set(&vp->vaddr_bitmap, btmp_idx + cnt, true);
            cnt++;
        }
        vaddr_ret = vaddr;
    }
    return vaddr_ret;
}

static void vaddr_release(struct v_pool* vp, void* vaddr, int pg_cnt)
{
    uint32_t vaddr_val = (uint32_t)vaddr;
    int btmp_idx = 0;

    btmp_idx = (vaddr_val - vp->vaddr_start) / PG_SIZE;

    //* for debugging
    TRACE_STR("vaddr release:btmp_idx=0x");
    TRACE_INT(btmp_idx);
    TRACE_STR(", cnt=0x");
    TRACE_INT(pg_cnt);
    TRACE_STR("\n");
    //*/

    bitmap_release(&vp->vaddr_bitmap, btmp_idx, pg_cnt);

}

static void* palloc(struct p_pool* pp)
{
    void* paddr;
    int btmp_idx = -1;

    btmp_idx = bitmap_acquire(&pp->paddr_bitmap, 1);

    //* for debugging
    TRACE_STR("palloc:btmp_idx=0x");
    TRACE_INT(btmp_idx);
    TRACE_STR("\n");
    //*/

    if (-1 == btmp_idx)
    {
        return NULL;
    }

    paddr = (void*)pp->paddr_start + (btmp_idx * PG_SIZE);
    return paddr;
}

static void pfree(struct p_pool* pp, void* paddr)
{
    uint32_t paddr_val = (uint32_t)paddr;
    int btmp_idx = 0;

    btmp_idx = (paddr_val - pp->paddr_start) / PG_SIZE;

    //* for debugging
    TRACE_STR("pfree:btmp_idx=0x");
    TRACE_INT(btmp_idx);
    TRACE_STR("\n");
    //*/

    bitmap_release(&pp->paddr_bitmap, btmp_idx, 1);
}

static void page_table_add(void* vaddr, void* paddr)
{
    uint32_t vaddr_val = (uint32_t)vaddr;
    uint32_t paddr_val = (uint32_t)paddr;
    uint32_t* pde = pde_ptr(vaddr);
    uint32_t* pte = pte_ptr(vaddr);
    uint32_t pt_paddr;

    if (!(*pde & 0x00000001)) {
        // page table always in kernel space
        pt_paddr = (uint32_t)palloc(&k_p_pool);
        *pde = (pt_paddr | PG_US_U | PG_RW_W | PG_P_1);
        memset((void*)((int)pte & 0xfffff000), 0, PG_SIZE);
    }
    *pte = (paddr_val | PG_US_U | PG_RW_W | PG_P_1);

    // update page table
    uint32_t cr3;
    asm volatile (
        "mov %%cr3, %0;" // read from CR3
        "mov %0, %%cr3;" // write back to CR3
        : "=r"(cr3)
        :
        : "memory"
   );
}

static void page_table_remove(void* vaddr)
{
    uint32_t* pte = pte_ptr(vaddr);
    *pte &= ~PG_P_1;
    asm volatile ("invlpg %0"::"m" (vaddr):"memory"); // update TLB
}

static void* page_acquire(struct v_pool* vp, struct p_pool* pp,\
    void* vaddr, int pg_cnt)
{
    void* vaddr_ret = NULL;
    void* paddr = NULL;

    // acquire virtual address
    mutex_lock(&vp->mlock);
    vaddr_ret = vaddr_acquire(vp, vaddr, pg_cnt);
    mutex_unlock(&vp->mlock);

    if (NULL == vaddr_ret) {
        return NULL;
    }

    // acquire physical address
    int n = 0;
    uint8_t* vaddr_idx = vaddr_ret;
    while (n < pg_cnt)
    {
        mutex_lock(&pp->mlock);
        paddr = palloc(pp);
        mutex_unlock(&pp->mlock);

        if (NULL == paddr) {
            return NULL;
        }

        // add into the page table
        mutex_lock(&mlock_pgtable);
        page_table_add(vaddr_idx, paddr);
        mutex_unlock(&mlock_pgtable);

        vaddr_idx += PG_SIZE;
        n++;
    }

    return vaddr_ret;
}

static void page_release(struct v_pool* vp, struct p_pool* pp,\
    void* vaddr, int pg_cnt)
{
    void* vaddr_start = vaddr;
    void* paddr = NULL;

    int n = 0;
    uint8_t* vaddr_idx = vaddr_start;
    while (n < pg_cnt)
    {
        // free physical memory
        paddr = (void*)addr_v2p(vaddr_idx);

        mutex_lock(&pp->mlock);
        pfree(pp, paddr);
        mutex_unlock(&pp->mlock);

        // remove page table
        mutex_lock(&mlock_pgtable);
        page_table_remove(vaddr_idx);
        mutex_unlock(&mlock_pgtable);

        vaddr_idx += PG_SIZE;
        n++;
    }

    // release virtual memory
    mutex_lock(&vp->mlock);
    vaddr_release(vp, vaddr_start, pg_cnt);
    mutex_unlock(&vp->mlock);
}

static void pool_init(uint32_t mem_total_size)
{
    TRACE_STR("memory total size: ");
    TRACE_INT(mem_total_size);
    TRACE_STR("\n");

    // calculate the physical memory size for kernel and user.
    uint32_t page_table_size = PG_SIZE * 256;
    uint32_t used_mem = page_table_size + 0x100000;
    uint32_t free_mem = mem_total_size - used_mem;
    uint32_t all_free_pages = free_mem / PG_SIZE;
    uint32_t kernel_free_pages = all_free_pages / 2;
    // The kernel physical space is limited to 1 GB = 0x40000 pages.
    if (kernel_free_pages > 0x40000) {
        kernel_free_pages = 0x40000;
    }
    uint32_t user_free_pages = all_free_pages - kernel_free_pages;

    TRACE_STR("kernel free pages: ");
    TRACE_INT(kernel_free_pages);
    TRACE_STR("\n");
    TRACE_STR("user free pages: ");
    TRACE_INT(user_free_pages);
    TRACE_STR("\n");

    // physical pool
    uint32_t kp_start = used_mem;
    uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE;
    k_p_pool.paddr_start = kp_start;
    u_p_pool.paddr_start = up_start;
    k_p_pool.size = kernel_free_pages * PG_SIZE;
    u_p_pool.size = user_free_pages * PG_SIZE;

    TRACE_STR("kernel physical pool start at ");
    TRACE_INT(k_p_pool.paddr_start);
    TRACE_STR("\n");
    TRACE_STR("user physical pool start at ");
    TRACE_INT(u_p_pool.paddr_start);
    TRACE_STR("\n");

    // physical pool bitmap
    uint32_t kbm_length = kernel_free_pages / 8;
    uint32_t ubm_length = user_free_pages / 8;
    struct bitmap* btmp;
    void* btmp_addr;
    btmp = &k_p_pool.paddr_bitmap;
    btmp_addr = (void*)PADDR_BITMAP_BASE;
    bitmap_init(btmp, btmp_addr, kbm_length);
    bitmap_reset(btmp);

    btmp = &u_p_pool.paddr_bitmap;
    btmp_addr = (void*)PADDR_BITMAP_BASE + kbm_length;
    bitmap_init(btmp, btmp_addr, ubm_length);
    bitmap_reset(btmp);

    TRACE_STR("kernel physical pool bitmap: ");
    TRACE_INT((uint32_t)k_p_pool.paddr_bitmap.bits);
    TRACE_STR("\n");
    TRACE_STR("user physical pool bitmap: ");
    TRACE_INT((uint32_t)u_p_pool.paddr_bitmap.bits);
    TRACE_STR("\n");

    // phusical pool lock
    mutex_init(&k_p_pool.mlock);
    mutex_init(&u_p_pool.mlock);

    // virtual pool
    k_v_pool.vaddr_start = K_HEAP_START;

    TRACE_STR("kernel virtual pool start at ");
    TRACE_INT(k_v_pool.vaddr_start);
    TRACE_STR("\n");

    // virtual pool bitmap
    btmp = &k_v_pool.vaddr_bitmap;
    btmp_addr = (void*)K_VADDR_BITMAP_BASE;
    bitmap_init(btmp, btmp_addr, 0x8000);
    bitmap_reset(btmp);

    TRACE_STR("kernel virtual pool bitmap: ");
    TRACE_INT((uint32_t)k_v_pool.vaddr_bitmap.bits);
    TRACE_STR("\n");

    // virtual pool lock
    mutex_init(&k_v_pool.mlock);
}

static struct mem_block* arena2block(struct arena* a, uint32_t idx)
{
  return (struct mem_block*)\
    ((uint32_t)a + sizeof(struct arena) + idx * a->p_mem_block->block_size);
}

static struct arena* block2arena(struct mem_block* b)
{
   return (struct arena*)((uint32_t)b & 0xfffff000);
}

static void* mem_acquire(struct v_pool* vp, struct p_pool* pp,\
    struct mem_block_desc* mblock, uint32_t mblock_idx)
{
    struct arena* a;
    struct mem_block* b;

    // new arena for memory block
    if (list_empty(&mblock[mblock_idx].free_list)) {
        a = (struct arena*)page_acquire(vp, pp, NULL, 1);
        if (NULL == a) {
            return NULL;
        }
        memset(a, 0, PG_SIZE);
        a->p_mem_block = &mblock[mblock_idx];
        a->cnt = mblock[mblock_idx].blocks_per_arena;
        a->is_page_cnt = false;

        int idx = 0;
        for (idx = 0; idx < mblock[mblock_idx].blocks_per_arena; idx++)
        {
            b= arena2block(a, idx);
            list_append(&a->p_mem_block->free_list, &b->free_elem);
        } // for
    } // if

    b = elem2entry(struct mem_block, free_elem,\
        list_pop(&(mblock[mblock_idx].free_list)));

    memset(b, 0, mblock[mblock_idx].block_size);
    a = block2arena(b);
    a->cnt--;
    return (void*)b;
}

static void mem_release(struct v_pool* vp, struct p_pool* pp, void* vaddr)
{
    struct mem_block* b = vaddr;
    struct arena* a = block2arena(b);

    list_append(&a->p_mem_block->free_list, &b->free_elem);
    a->cnt++;

    // Check whether all memory blocks in this arena are free
    // if so, release the arena.
    if (a->cnt == a->p_mem_block->blocks_per_arena) {
        int idx = 0;
        for (idx = 0; idx < a->p_mem_block->blocks_per_arena; idx++)
        {
            b= arena2block(a, idx);
            list_remove(&b->free_elem);
        }
        page_release(vp, pp, a, 1);
    }
}

//=========================
// external functions
//=========================

void* page_malloc(enum pool_flags pf, void* vaddr, int pg_cnt)
{
    void* vaddr_start = NULL;

    if (PF_KERNEL == pf) {
        vaddr_start = page_acquire(&k_v_pool, &k_p_pool, vaddr, pg_cnt);
    } else if (PF_USER == pf) {
        // TODO: user process
    } else {
        // invalid pool flag
        return NULL;
    }

    return vaddr_start;
}

void page_free(enum pool_flags pf, void* vaddr, int pg_cnt)
{

    if (PF_KERNEL == pf) {
        page_release(&k_v_pool, &k_p_pool, vaddr, pg_cnt);
    } else if (PF_USER == pf) {
        // TODO: user process
    } else {
        // invalid pool flag
        return;
    }

    return;
}

void* sys_malloc(uint32_t size)
{
    struct mem_block_desc*  mblock = NULL;
    struct v_pool*          vp;
    struct p_pool*          pp;
    void*                   vaddr = NULL;

    // TODO: determine whether it is in kernel space or user space
    mblock = k_mem_block;
    vp = &k_v_pool;
    pp = &k_p_pool;

    if (size > 1024) {
        uint32_t page_cnt = DIV_ROUND_UP(size + sizeof(struct arena), PG_SIZE);
        struct arena* a = (struct arena*)page_acquire(vp, pp, NULL, page_cnt);
        if (NULL == a) {
            return NULL;
        }
        memset(a, 0, page_cnt * PG_SIZE);
        //set arena info
        a->p_mem_block = NULL;
        a->cnt = page_cnt;
        a->is_page_cnt = true;
        vaddr = (void*)(a+1);

        //* for debugging
        uint32_t* p_arena = (uint32_t*)a;
        TRACE_STR("sys_malloc:arena=0x");
        TRACE_INT((uint32_t)p_arena);
        TRACE_STR(", cnt=0x");
        TRACE_INT(*(p_arena+1));
        TRACE_STR("\n");
        //*/

    } else { // using memory block
        // find a suitable size
        uint8_t mblock_idx = 0;
        for (mblock_idx=0; mblock_idx<MEM_BLOCK_CNT; mblock_idx++)
        {
            if (size <= mblock[mblock_idx].block_size) {
                break;
            }
        } //for

        mutex_lock(&mblock[mblock_idx].mlock);
        vaddr = mem_acquire(vp, pp, mblock, mblock_idx);
        mutex_unlock(&mblock[mblock_idx].mlock);
    } // if-else (size > 1024)

    return vaddr;
}

void sys_free(void* vaddr)
{
    struct v_pool*  vp;
    struct p_pool*  pp;
    struct arena*   a = block2arena((struct mem_block*)vaddr);

    //* for debugging
    uint32_t* p_arena = (uint32_t*)a;
    TRACE_STR("sys_free:arena=0x");
    TRACE_INT((uint32_t)p_arena);
    TRACE_STR(", cnt=0x");
    TRACE_INT(*(p_arena+1));
    TRACE_STR("\n");
    //*/

    // TODO: determine whether it is in kernel space or user space
    vp = &k_v_pool;
    pp = &k_p_pool;

    if (a->is_page_cnt) {
        page_release(vp, pp, a, a->cnt);
    } else { // memory block
        mutex_lock(&a->p_mem_block->mlock);
        mem_release(vp, pp, vaddr);
        mutex_unlock(&a->p_mem_block->mlock);
    }
}

void mem_block_init(struct mem_block_desc* p_mem_block)
{
    int idx;
    uint32_t block_size = 16;

    for (idx=0; idx<MEM_BLOCK_CNT; idx++)
    {
        p_mem_block[idx].block_size = block_size;
        p_mem_block[idx].blocks_per_arena = \
            (PG_SIZE - sizeof(struct arena)) / block_size;
        list_init(&p_mem_block[idx].free_list);

        // lock init
        mutex_init(&p_mem_block[idx].mlock);

        block_size *= 2;
    }
}

void mem_init()
{
    TRACE_STR("mem_init()\n");
    uint32_t mem_total_size = (*(uint32_t*)MEM_SIZE_ADDR);
    pool_init(mem_total_size);
    mem_block_init(k_mem_block);
    mutex_init(&mlock_pgtable);
}


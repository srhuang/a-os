#include "process.h"
#include "stdint.h"
#include "printk.h"
#include "ide.h"
#include "memory.h"
#include "file.h"
#include "math.h"
#include "stddef.h"
#include "string.h"
#include "kernel.h"

//=========================
// debugging
//=========================
#define DEBUG

#ifdef DEBUG
    #define pr_debug(fmt, ...) printk(fmt, ##__VA_ARGS__)
#else
    #define pr_debug(fmt, ...) do { } while (0)
#endif

//=========================
// internal struct
//=========================
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Word;

struct Elf32_Ehdr {
    unsigned char e_ident[16];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
};

struct Elf32_Phdr{
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
 } Elf32_Phdr;

enum segment_type {
    PT_NULL,
    PT_LOAD,
    PT_DYNAMIC,
    PT_INTERP,
    PT_NOTE,
    PT_SHLIB,
    PT_PHDR
};

struct tss {
    uint32_t backlink;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t (*eip) (void);
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint32_t trace;
    uint32_t io_base;
};

struct gdt_desc {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  attr_low;
    uint8_t  limit_attr_high;
    uint8_t  base_high;
};

struct kstack_ret {
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    void (*eip) (void);
    uint32_t cs;
    uint32_t esp;
    uint32_t ss;
};

//=========================
// global variable
//=========================
static struct tss usr_tss;

//=========================
// internal functions
//=========================
static struct gdt_desc make_gdt_table( \
    uint32_t base, uint32_t limit, uint32_t attr)
{
    struct gdt_desc desc;

    pr_debug("base=0x%x, limit=0x%x, attr=0x%x\n", base, limit, attr);

    desc.limit_low = limit & 0x0000FFFF;
    desc.base_low = base & 0x0000FFFF;
    desc.base_mid = (base & 0x00FF0000) >> 16;
    desc.attr_low = (attr & 0x0000FF00) >> 8;
    desc.limit_attr_high = ((limit & 0x000F0000) + (attr & 0x00F00000)) >> 16;
    desc.base_high = base >> 24;

    return desc;
}

static void segment_load(int32_t fd, \
    uint32_t offset, uint32_t size, uint32_t vaddr)
{
    pr_debug("%s:offset=%d, size=%d, vaddr=0x%x\n", \
        __func__, offset, size, vaddr);
    // calculate the count of pages
    uint32_t vaddr_max = vaddr + size - 1;
    uint32_t pg_cnt = \
        (((vaddr_max & 0xFFFFF000) - (vaddr & 0xFFFFF000)) >> 12) + 1;
    //pr_debug("pg_cnt:%d\n", pg_cnt);

    // allocate user pages
    page_malloc(PF_USER, (void*)vaddr, pg_cnt);

    // load the text
    sys_lseek(fd, offset, SEEK_SET);
    sys_read(fd, (void*)vaddr, size);
}

static void vaddr_create(struct task_struct* task)
{
    // user space virtual address
    struct v_pool* vp = page_malloc(PF_KERNEL, NULL, 1);
    //pr_debug("%s:vp=0x%x\n", __func__, vp);
    vp->vaddr_start = U_VADDR_START;
    // user virtual address bitmap
    uint32_t free_pages = (0xC0000000 - U_VADDR_START) / PG_SIZE;
    uint32_t btmp_len = free_pages / 8;
    uint32_t btmp_pg_cnt = DIV_ROUND_UP(btmp_len, PG_SIZE);
    uint8_t* btmp_addr = page_malloc(PF_KERNEL, NULL, btmp_pg_cnt);
    struct bitmap* btmp = &vp->vaddr_bitmap;
    bitmap_init(btmp, btmp_addr, btmp_len);
    bitmap_reset(btmp);
    // virtual address lock
    mutex_init(&vp->mlock);
    //pr_debug("%s:btmp_addr=0x%x, btmp_pg_cnt=%d\n", \
        __func__, btmp_addr, btmp_pg_cnt);
    task->u_v_pool = vp;

    // for memory block
    struct mem_block_desc* mblock = \
        (struct mem_block_desc*)((uint8_t*)vp + sizeof(struct v_pool));
    //pr_debug("%s:mblock=0x%x\n", __func__, mblock);
    mem_block_init(mblock);
    task->mblock = mblock;
}

static void page_create(struct task_struct* task, uint32_t start_idx)
{
    // page table
    uint32_t* pde_child = page_malloc(PF_KERNEL, NULL, 1);
    // copy the kernel space in page table (1GB)
    uint32_t* pde_parent = (uint32_t*)0xFFFFF000;
    memcpy(pde_child + start_idx, \
        pde_parent + start_idx, PG_SIZE - (start_idx * 4));
    // get pde physical address
    uint32_t vaddr = (uint32_t)pde_child;
    uint32_t* ptr = (uint32_t*)(0xFFC00000 + \
        ((vaddr & 0xFFC00000) >> 10) + ((vaddr & 0x003FF000) >> 12) * 4);
    uint32_t paddr = (*ptr & 0xfffff000);
    // update pde physical address
    *(pde_child + 1023) = paddr | PG_US_U | PG_RW_W | PG_P_1;
    //pr_debug("%s:pde vaddr=0x%x, paddr=0x%x\n", __func__, vaddr, paddr);

    task->pgdir_paddr = paddr;
    task->pgdir_vaddr = vaddr;
}

static void process_fs(uint32_t sec_start, const char* path)
{
    struct ide_hd* hd = &g_ide_ch[0].dev[0];

    // get file size from ELF
    void* buf = sys_malloc(SECTOR_SIZE);
    ide_read(hd, sec_start, buf, 1);
    struct Elf32_Ehdr* elf = (struct Elf32_Ehdr*)buf;
    uint32_t file_size = elf->e_shoff + (elf->e_shentsize * elf->e_shnum);
    sys_free(buf);
    /*/
    pr_debug("Start of section headers:%d\n", elf->e_shoff);
    pr_debug("Size of section headers:%d\n", elf->e_shentsize);
    pr_debug("Number of section headers:%d\n", elf->e_shnum);
    //*/

    // write file to file system
    buf = sys_malloc(DIV_ROUND_UP(file_size, SECTOR_SIZE) * SECTOR_SIZE);
    ide_read(hd, sec_start, buf, DIV_ROUND_UP(file_size, SECTOR_SIZE));
    int32_t fd = sys_open(path, O_RDWR | O_TRUNC);
    if (-1 == fd) {
        fd = sys_open(path, O_CREATE | O_RDWR | O_TRUNC);
    }
    sys_write(fd, buf, file_size);
    sys_free(buf);
    pr_debug("%s:Write File(%s) size:%d\n", __func__, path, file_size);
}

static int32_t process_load(const char* path)
{
    int32_t ret = -1;
    int32_t fd = sys_open(path, O_RDONLY);
    if (fd == -1) {
        return -1;
    }

    // get elf
    void* buf = sys_malloc(sizeof(struct Elf32_Ehdr));
    sys_read(fd, buf, sizeof(struct Elf32_Ehdr));
    struct Elf32_Ehdr* elf = (struct Elf32_Ehdr*)buf;
    uint16_t phnum = elf->e_phnum;
    uint16_t phentsize = elf->e_phentsize;
    uint32_t phoff = elf->e_phoff;
    uint32_t entry = elf->e_entry;
    sys_free(buf);
    /*/
    pr_debug("Program header number:%d\n", phnum);
    pr_debug("Program header size:%d\n", phentsize);
    pr_debug("Program header offset:%d\n", phoff);
    //*/

    // parsing program header
    buf = sys_malloc(sizeof(struct Elf32_Phdr));
    uint32_t idx;
    for (idx = 0; idx < phnum; idx++)
    {
        // get program header
        sys_lseek(fd, phoff, SEEK_SET);
        sys_read(fd, buf, sizeof(struct Elf32_Phdr));
        struct Elf32_Phdr* ph = (struct Elf32_Phdr*)buf;
        /*/
        pr_debug("Program header type:%d\n", ph->p_type);
        pr_debug("Program header offset:%d\n", ph->p_offset);
        pr_debug("Program header vaddr:0x%x\n", ph->p_vaddr);
        pr_debug("Program header size:0x%x\n", ph->p_filesz);
        //*/

        // load segment
        if (PT_LOAD == ph->p_type) {
            segment_load(fd, ph->p_offset, ph->p_filesz, ph->p_vaddr);
        }

        // next program header
        phoff += phentsize;
    } // for
    sys_free(buf);

    // close fd
    sys_close(fd);

    // return address of entry point
    return entry;
}

static void process_create(struct task_struct* child)
{
    // create user space virtual address
    vaddr_create(child);
    // copy the user space virtual bitmap
    struct task_struct* parent = kthread_current();
    uint8_t* p_btmp_addr = parent->u_v_pool->vaddr_bitmap.bits;
    uint32_t p_btmp_len = parent->u_v_pool->vaddr_bitmap.len;
    uint8_t* c_btmp_addr = child->u_v_pool->vaddr_bitmap.bits;
    uint32_t btmp_idx = \
        (U_HEAP_START - child->u_v_pool->vaddr_start) / PG_SIZE / 8;
    memcpy(c_btmp_addr + btmp_idx, p_btmp_addr, p_btmp_len);
    //pr_debug("%s:p_btmp_addr=0x%x, btmp_idx=0x%x, p_btmp_len=0x%x\n", \
        __func__, p_btmp_addr, btmp_idx, p_btmp_len);

    // page table
    page_create(child, 1);
}

static void process_start(void* entry)
{
    pr_debug("%s:entry=0x%x\n", __func__, entry);

    // user stack
    uint32_t* usr_stack = page_malloc(PF_USER, (void*)U_STACK_START, 1);

    // kernel stack
    struct task_struct* task = kthread_current();
    task->kstack = (uint32_t)task + PG_SIZE;
    task->kstack -= sizeof(struct kstack_ret);
    struct kstack_ret* sp = (struct kstack_ret*)task->kstack;
    sp->gs = 0;
    sp->ds = sp->es = sp->fs = SELECTOR_U_DATA;
    sp->eip = entry;
    sp->cs = SELECTOR_U_CODE;
    sp->esp = (uint32_t)usr_stack + PG_SIZE;
    sp->ss = SELECTOR_U_STACK;
    pr_debug("eip=0x%x, cs=0x%x, esp=0x%x, ss=0x%x\n", \
        sp->eip, sp->cs, sp->esp, sp->ss);

    // switch to ring 3
    asm volatile ("movl %0, %%esp;" : : "g" (sp));
    asm volatile ("pop %gs; pop %fs; pop %es; pop %ds;");
    asm volatile ("retf;");
}

//=========================
// external functions
//=========================
void tss_init()
{
    printk("%s +++\n", __func__);

    // tss GDT
    uint32_t tss_size = sizeof(usr_tss);
    memset(&usr_tss, 0, tss_size);
    usr_tss.io_base = tss_size;
    usr_tss.ss0 = SELECTOR_K_STACK;
    uint32_t attr = GDT_G_1 + GDT_D_16 + GDT_L_32 + GDT_AVL_0 + GDT_P_1 \
        + GDT_DPL_0 + GDT_S_HW + GDT_TYPE_TSS;
    *((struct gdt_desc*)GDT_TSS) = \
        make_gdt_table((uint32_t)&usr_tss, tss_size - 1, attr);

    // for user process code and data GDT
    attr = GDT_G_4K + GDT_D_32 + GDT_L_32 + GDT_AVL_0 + GDT_P_1 \
        + GDT_DPL_3 + GDT_S_SW + GDT_TYPE_CODE;
    *((struct gdt_desc*)GDT_U_CODE) = \
        make_gdt_table(0, 0xFFFFF, attr);

    attr = GDT_G_4K + GDT_D_32 + GDT_L_32 + GDT_AVL_0 + GDT_P_1 \
        + GDT_DPL_3 + GDT_S_SW + GDT_TYPE_DATA;
    *((struct gdt_desc*)GDT_U_DATA) = \
        make_gdt_table(0, 0xFFFFF, attr);

    uint64_t gdt_reg = ((uint64_t)(uint32_t)GDT_BASE << 16) | GDT_LIMIT;
    asm volatile ("lgdt %0" : : "m" (gdt_reg));
    asm volatile ("ltr %w0" : : "r" (SELECTOR_TSS));

    printk("%s ---\n", __func__);
}

void process_init()
{
    printk("%s +++\n", __func__);

    // user init
    process_fs(USR_START_SECTOR, USR_INIT_PATH);
    int32_t entry = process_load(USR_INIT_PATH);
    if (-1 == entry) {
        printk("%s: process load fail.\n", __func__);
    }
    pr_debug("%s:entry:0x%x\n", __func__, entry);

    // create task
    struct task_struct* task = \
        kthread_create(process_start, (void*)entry, "init");
    process_create(task);

    // run task
    kthread_run(task);

    printk("%s ---\n", __func__);
}

void process_switch(struct task_struct* task)
{
    // update tss esp for user process
    usr_tss.esp0 = (uint32_t)task + PG_SIZE;
}


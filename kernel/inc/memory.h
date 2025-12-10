#ifndef __KERNEL_INC_MEMORY_H
#define __KERNEL_INC_MEMORY_H
#include "stdint.h"
#include "bitmap.h"
#include "list.h"
#include "lock.h"

//=========================
// define
//=========================
#define PG_SIZE (4096)
#define PG_P_0  (0x0 << 0)
#define PG_P_1  (0x1 << 0)
#define PG_RW_R (0x0 << 1)
#define PG_RW_W (0x1 << 1)
#define PG_US_S (0x0 << 2)
#define PG_US_U (0x1 << 2)

enum pool_flags {
    PF_KERNEL = 1,
    PF_USER = 2
};

// the address of memory size
#define MEM_SIZE_ADDR       (0x900)

// each 4KB can maintain 128MB, 128KB(0x20000) can maintain 4GB.
#define PADDR_BITMAP_BASE   (0xC005E000)

// each 4KB can maintain 128MB, 128KB(0x20000) can maintain 4GB.
#define VADDR_BITMAP_BASE   (0xC007E000)

#define PDE_IDX(addr)       ((addr & 0xFFC00000) >> 22)
#define PTE_IDX(addr)       ((addr & 0x003FF000) >> 12)

// for 1024/512/256/128/64/32/16 byte block types
#define MEM_BLOCK_CNT       (7)

// kernel page table
#define K_PGDIR_PADDR       (0x00100000)

// kernel thread virtual address
#define K_HEAP_START        (0xC0100000)
#define U_HEAP_START        (0x00400000)

// user process virtual address
#define U_VADDR_START       (0x00010000)
#define U_STACK_START       (0xC0000000 - 0x1000)

//=========================
// struct
//=========================
// The system contains only one single physical pool.
struct p_pool {
   uint32_t         paddr_start;
   uint32_t         size;
   struct bitmap    paddr_bitmap;
   struct mutex     mlock;
};

// The system contains multiple virtual pools.
struct v_pool {
    uint32_t        vaddr_start;
    struct bitmap   vaddr_bitmap;
    struct mutex    mlock;
};

struct mem_block_desc {
    uint32_t        block_size;
    uint32_t        blocks_per_arena;
    struct list     free_list;
    struct mutex    mlock;
};

//=========================
// external variable
//=========================
extern struct v_pool u_v_pool;
extern struct mem_block_desc k_mem_block[MEM_BLOCK_CNT];

//=========================
// function
//=========================
void*   page_malloc(enum pool_flags pf, void* vaddr, int pg_cnt);
void    page_free(enum pool_flags pf, void* vaddr, int pg_cnt);
void*   sys_malloc(uint32_t size);
void    sys_free(void* vaddr);
void    mem_init(void);
void    mem_block_init(struct mem_block_desc* p_mem_block);

#endif

#ifndef __KERNEL_INC_LOCK_H
#define __KERNEL_INC_LOCK_H
#include "stdint.h"
#include "list.h"
#include "thread.h"

//=========================
// define
//=========================
static inline uint32_t xchg(volatile uint32_t *ptr, int x)
{
    asm volatile("xchg %0,%1"
                : "=r" (x), "=m" (*ptr)
                : "0" (x), "m" (*ptr)
                : "memory");
    return x;
}

//=========================
// struct
//=========================
struct spinlock {
    uint32_t    value;
};

struct semaphore {
    struct spinlock slock;
    uint32_t        count;
    struct list     wait_list;
};

struct mutex {
    struct spinlock     slock;
    struct task_struct* owner;
    struct list         wait_list;
};

//=========================
// external variable
//=========================


//=========================
// function
//=========================
void spin_lock_init(struct spinlock*);
void spin_lock(struct spinlock*);
void spin_unlock(struct spinlock*);
void sema_init(struct semaphore*, uint32_t);
void sema_down(struct semaphore*);
void sema_up(struct semaphore*);
void mutex_init(struct mutex*);
void mutex_lock(struct mutex*);
void mutex_unlock(struct mutex*);

#endif

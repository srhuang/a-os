#include "lock.h"
#include "stddef.h"

//=========================
// debugging
//=========================
#define DEBUG (1)
#define TRACE_STR(x) do {if(DEBUG) put_str(x);} while(0)
#define TRACE_INT(x) do {if(DEBUG) put_int(x);} while(0)

//=========================
// internal struct
//=========================
struct lock_waiter {
    struct task_struct* task;
    struct list_elem    wait_tag;
};

//=========================
// global variable
//=========================


//=========================
// internal functions
//=========================


//=========================
// external functions
//=========================
void spin_lock_init(struct spinlock* lock)
{
    lock->value = 0;
}

void spin_lock(struct spinlock* lock)
{
    while (1)
    {
        // x86 atomic instruction: xchg
        if (!xchg(&lock->value, 1)) {
            break;
        }

        // wait until lock released
        while (lock->value)
        {
            kthread_yield();
        }
    }
}

void spin_unlock(struct spinlock* lock)
{
    lock->value = 0;
}

void sema_init(struct semaphore* sema, uint32_t value)
{
    spin_lock_init(&sema->slock);
    sema->count = value;
    list_init(&sema->wait_list);
}

void sema_down(struct semaphore* sema)
{
    struct task_struct* task = kthread_current();
    struct lock_waiter waiter;
    waiter.task  = task;

    while (1)
    {
        // lock
        spin_lock(&sema->slock);

        if (sema->count > 0) {
            sema->count--;

            // unlock
            spin_unlock(&sema->slock);
            break;
        } else {
            list_append(&sema->wait_list, &waiter.wait_tag);

            // unlock
            spin_unlock(&sema->slock);
            kthread_block(TASK_BLOCKED);
        }
    }// while
}

void sema_up(struct semaphore* sema)
{
    // lock
    spin_lock(&sema->slock);

    sema->count++;

    // wakeup waiting task
    if (!list_empty(&sema->wait_list))
    {
        struct list_elem* waiter_tag = list_pop(&sema->wait_list);
        struct lock_waiter* waiter = \
            elem2entry(struct lock_waiter, wait_tag, waiter_tag);
        struct task_struct* task = waiter->task;
        kthread_unblock(task);
    }

    // unlock
    spin_unlock(&sema->slock);
}

void mutex_init(struct mutex* mlock)
{
    spin_lock_init(&mlock->slock);
    mlock->owner = NULL;
    list_init(&mlock->wait_list);
}

void mutex_lock(struct mutex* mlock)
{
    struct task_struct* task = kthread_current();
    struct lock_waiter waiter;
    waiter.task = task;

    while (1)
    {
        // lock
        spin_lock(&mlock->slock);

        if (NULL == mlock->owner) {
            mlock->owner = task;

            // unlock
            spin_unlock(&mlock->slock);
            break;
        } else {
            list_append(&mlock->wait_list, &waiter.wait_tag);

            // unlock
            spin_unlock(&mlock->slock);
            kthread_block(TASK_BLOCKED);
        }
    }// while
}

void mutex_unlock(struct mutex* mlock)
{
    // lock
    spin_lock(&mlock->slock);

    mlock->owner = NULL;

    // wakeup waiting task
    if (!list_empty(&mlock->wait_list))
    {
        struct list_elem* waiter_tag = list_pop(&mlock->wait_list);
        struct lock_waiter* waiter = \
            elem2entry(struct lock_waiter, wait_tag, waiter_tag);
        struct task_struct* task = waiter->task;
        kthread_unblock(task);
    }

    // unlock
    spin_unlock(&mlock->slock);
}


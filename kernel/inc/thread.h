#ifndef __KERNEL_INC_THREAD_H
#define __KERNEL_INC_THREAD_H
#include "list.h"

//=========================
// define
//=========================
typedef void (*threadfn) (void*);

enum task_status {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};

//=========================
// struct
//=========================
struct task_struct
{
    // MUST be the first member
    void*       kstack;

    // scheduler related
    enum        task_status status;
    uint32_t    time_slice;
    struct      list_elem task_tag;
    struct      list_elem task_all_tag;

    // task name
    char        name[16];

    // MUST be the last member
    uint32_t    stack_magic;
};

// For first time execution
struct kstack_switch
{
    uint32_t        ebp;
    uint32_t        ebx;
    uint32_t        edi;
    uint32_t        esi;

    // return to kernel_thread()
    void            (*kthread) (threadfn fn, void* func_arg);
    void*           unused_retaddr;
    threadfn        fn;
    void*           fn_arg;
};

//=========================
// external variable
//=========================
extern struct list task_ready_list;
extern struct list task_all_list;
extern struct task_struct* g_idle_task;

//=========================
// function
//=========================
struct task_struct*     kthread_current(void);
struct task_struct*     kthread_create(threadfn fn, void* fn_arg, char* name);
void                    kthread_run(struct task_struct* task);
void                    kthread_exit(void);
void                    kthread_block(enum task_status stat);
void                    kthread_unblock(struct task_struct* task);
void                    kthread_yield(void);
void                    kthread_init(void);

#endif


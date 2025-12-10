#include "sched.h"
#include "interrupt.h"
#include "thread.h"
#include "process.h"
#include "memory.h"

//=========================
// debugging
//=========================
#define DEBUG (0)
#define TRACE_STR(x) do {if(DEBUG) put_str(x);} while(0)
#define TRACE_INT(x) do {if(DEBUG) put_int(x);} while(0)

//=========================
// internal struct
//=========================


//=========================
// global variable
//=========================


//=========================
// internal functions
//=========================


//=========================
// external functions
//=========================
void schedule(void)
{
    // Round-Robin Scheduling
    struct task_struct* cur_task = kthread_current();
    if (TASK_RUNNING == cur_task->status) {
        cur_task->status        = TASK_READY;
        cur_task->time_slice    = SCHED_RR_TIME_SLICE;
        list_append(&task_ready_list, &cur_task->task_tag);
    }

    // check the ready list
    if (list_empty(&task_ready_list)) {
        kthread_unblock(g_idle_task);
    }

    // pick up the task form ready list
    struct list_elem* next_task_tag = list_pop(&task_ready_list);
    struct task_struct* next_task = \
        elem2entry(struct task_struct, task_tag, next_task_tag);
    next_task->status = TASK_RUNNING;

    TRACE_STR("next task: ");
    TRACE_STR(next_task->name);
    TRACE_STR("\n");

    // switch page table
    uint32_t page_table = next_task->pgdir_paddr;
    asm volatile ("movl %0, %%cr3" : : "r" (page_table) : "memory");

    // update tss esp0 (user process only)
    if (K_PGDIR_PADDR != page_table) {
        process_switch(next_task);
    }

    switch_to(cur_task, next_task);
}


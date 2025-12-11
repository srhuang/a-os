#include "thread.h"
#include "print.h"
#include "stddef.h"
#include "memory.h"
#include "string.h"
#include "list.h"
#include "interrupt.h"
#include "sched.h"
#include "file.h"

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
struct list task_ready_list;
struct list task_all_list;
struct task_struct* g_main_task;
struct task_struct* g_idle_task;

//=========================
// internal functions
//=========================
static void kernel_thread(threadfn fn, void* fn_arg)
{
    TRACE_STR("kernel_thread intr status: ");
    TRACE_INT((int)intr_get_status());
    TRACE_STR("\n");

    intr_set_status(true);
    fn(fn_arg);
}

static void main_thread_init(void)
{
    uint32_t esp;
    asm ("mov %%esp, %0" : "=g" (esp));

    // init task_struct
    g_main_task = kthread_current();
    memset(g_main_task, 0, sizeof(struct task_struct));
    g_main_task->kstack = esp;
    strcpy(g_main_task->name, "main");
    g_main_task->stack_magic = 0x19880802;

    // init file descriptor
    g_main_task->open_fd[0] = stdin_no;
    g_main_task->open_fd[1] = stdout_no;
    g_main_task->open_fd[2] = stderr_no;
    uint32_t task_fd_idx = 3;
    while (task_fd_idx < MAX_FD_PER_TASK) {
        g_main_task->open_fd[task_fd_idx] = -1;
        task_fd_idx++;
    }

    // start for scheduling
    g_main_task->status = TASK_RUNNING;
    g_main_task->time_slice = SCHED_RR_TIME_SLICE;
    list_append(&task_all_list, &g_main_task->task_all_tag);
}

static void idle_thread(void* arg)
{
    uint32_t count = 0;
    while(1) {

        /*
        set_cursor(80);
        put_str("idle:0x");
        put_int(count++);
        //*/

        kthread_block(TASK_BLOCKED);
        asm volatile ("sti; hlt" : : : "memory");
    }
}

//=========================
// external functions
//=========================
struct task_struct* kthread_current(void)
{
    uint32_t esp;
    asm ("mov %%esp, %0" : "=g" (esp));

    // get the start address of PCB
    return (struct task_struct*)(esp & 0xfffff000);
}

struct task_struct* kthread_create(threadfn fn, void* fn_arg, char* name)
{
    struct task_struct* task = page_malloc(PF_KERNEL, NULL, 1);

    TRACE_STR("task_struct: 0x");
    TRACE_INT((uint32_t)task);
    TRACE_STR("\n");

    // init task_struct
    memset(task, 0, sizeof(struct task_struct));
    task->kstack = (uint32_t)task + PG_SIZE;
    strcpy(task->name, name);
    task->stack_magic = 0x19880802;

    //init kstack
    task->kstack -= sizeof(struct kstack_switch);
    struct kstack_switch* ks_sw = (struct kstack_switch*)task->kstack;
    ks_sw->ebp      = 0;
    ks_sw->ebx      = 0;
    ks_sw->edi      = 0;
    ks_sw->esi      = 0;
    ks_sw->kthread  = kernel_thread;
    ks_sw->fn       = fn;
    ks_sw->fn_arg   = fn_arg;

    // init file descriptor
    task->open_fd[0] = stdin_no;
    task->open_fd[1] = stdout_no;
    task->open_fd[2] = stderr_no;
    uint32_t task_fd_idx = 3;
    while (task_fd_idx < MAX_FD_PER_TASK) {
        task->open_fd[task_fd_idx] = -1;
        task_fd_idx++;
    }

    return task;
}

void kthread_run(struct task_struct* task)
{
    // backup status of interrupt
    bool intr_stat = intr_get_status();
    // disable interrupt
    intr_set_status(false);

    // for scheduler
    task->status        = TASK_READY;
    task->time_slice    = SCHED_RR_TIME_SLICE;
    list_append(&task_ready_list, &task->task_tag);
    list_append(&task_all_list, &task->task_all_tag);

    // restore status of interrupt
    intr_set_status(intr_stat);
}

void kthread_exit(void)
{
    // backup status of interrupt
    bool intr_stat = intr_get_status();
    // disable interrupt
    intr_set_status(false);

    struct task_struct* task = kthread_current();

    // for scheduler
    task->status = TASK_DIED;
    if (list_find(&task_ready_list, &task->task_tag)) {
        list_remove(&task->task_tag);
    }
    list_remove(&task->task_all_tag);

    // free page for PCB
    if (task != g_main_task)
    {
        page_free(PF_KERNEL, task, 1);
    }

    //descheduled
    schedule();

    // restore status of interrupt
    intr_set_status(intr_stat);
}

void kthread_block(enum task_status stat)
{
    // backup status of interrupt
    bool intr_stat = intr_get_status();
    // disable interrupt
    intr_set_status(false);

    if (TASK_BLOCKED == stat \
        || TASK_WAITING == stat \
        || TASK_HANGING == stat){
        // update the status
        struct task_struct* task = kthread_current();
        task->status = stat;

        //descheduled
        schedule();
    }

    // restore status of interrupt
    intr_set_status(intr_stat);
}

void kthread_unblock(struct task_struct* task)
{
    // backup status of interrupt
    bool intr_stat = intr_get_status();
    // disable interrupt
    intr_set_status(false);

    int stat = task->status;

    if (TASK_BLOCKED == stat \
        || TASK_WAITING == stat \
        || TASK_HANGING == stat){
        // ready to schedule
        task->status = TASK_READY;
        list_push(&task_ready_list, &task->task_tag);
    }

    // restore status of interrupt
    intr_set_status(intr_stat);
}

// Get back in line
void kthread_yield(void)
{
    // backup status of interrupt
    bool intr_stat = intr_get_status();
    // disable interrupt
    intr_set_status(false);

    struct task_struct* task = kthread_current();
    task->status = TASK_READY;
    list_append(&task_ready_list, &task->task_tag);

    //Reschedule
    schedule();

    // restore status of interrupt
    intr_set_status(intr_stat);
}

void kthread_init(void)
{
    list_init(&task_ready_list);
    list_init(&task_all_list);

    g_idle_task = kthread_create(idle_thread, NULL, "idle");
    kthread_run(g_idle_task);

    main_thread_init();
}


#include "init.h"
#include "memory.h"
#include "interrupt.h"
#include "syscall_sys.h"
#include "timer.h"
#include "thread.h"
#include "printk.h"

void kernel_init()
{
    mem_init();
    intr_init();
    syscall_init();
    timer_init();
    kthread_init();
    printk_init();

    // enable interrupt
    intr_set_status(true);
}

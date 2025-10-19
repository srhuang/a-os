#include "init.h"
#include "memory.h"
#include "interrupt.h"
#include "syscall_sys.h"

void kernel_init()
{
    mem_init();
    intr_init();
    syscall_init();
}

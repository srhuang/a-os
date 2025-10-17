#include "init.h"
#include "memory.h"
#include "interrupt.h"

void kernel_init()
{
    mem_init();
    intr_init();
}

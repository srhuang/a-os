#include "assert.h"
#include "interrupt.h"
#include "stdbool.h"
#include "printk.h"

void kernel_panic(char* filename, int line,\
    const char* func, const char* condition)
{
    intr_set_status(false);
    printk("\n!!!!! error !!!!!\n");
    printk("Filename: %s\n", filename);
    printk("Line: %d\n", line);
    printk("Function: %s\n", func);
    printk("Condition: %s\n", condition);
    while(1);
}


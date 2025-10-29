#include "printk.h"
#include "stdio.h"
#include "print.h"
#include "lock.h"

static struct mutex printk_lock;

void printk(const char* format, ...)
{
    va_list args;

    va_start(args, format);
    char buf[1024] = {0};
    vsprintf(buf, format, args);
    va_end(args);

    // print
    mutex_lock(&printk_lock);
    put_str(buf);
    mutex_unlock(&printk_lock);
}

void printk_init(void)
{
    mutex_init(&printk_lock);
}


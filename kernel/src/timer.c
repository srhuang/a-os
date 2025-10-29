#include "timer.h"
#include "print.h"
#include "io.h"
#include "interrupt.h"
#include "thread.h"
#include "sched.h"

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
uint32_t jiffies;

//=========================
// internal functions
//=========================
static void timer_set(int us)
{
    // set counter control
    uint8_t counter_ctl = \
        COUNTER_CH_0 + COUNTER_AC_ALL + COUNTER_MODE_2 + COUNTER_BIN;
    outb(COUNTER_CTL_PORT, counter_ctl);

    //set counter data
    uint16_t counter_data;
    // 0xFFFF * 1000000 / 1193180 = 54924 us
    if (us < 54924) {
        counter_data = US_TO_TICKS(us);
    } else {
        counter_data = 0xFFFF;
    }
    outb(CONTRER_PORT_0, (uint8_t)counter_data);
    outb(CONTRER_PORT_0, (uint8_t)(counter_data >> 8));
}

static void timer_handler(void)
{
    jiffies++;

    /*
    TRACE_STR("system jiffies: 0x");
    TRACE_INT(jiffies);
    TRACE_STR("\n");
    //*/

    struct task_struct* cur_task = kthread_current();
    if (0 == cur_task->time_slice) {
        schedule();
    } else {
        cur_task->time_slice--;
    }
}

//=========================
// external functions
//=========================
uint32_t msecs_to_jiffies(uint32_t ms)
{
    return (ms * 1000 / SYS_TICK_US);
}

uint32_t usecs_to_jiffies(uint32_t us)
{
    return (us / SYS_TICK_US);
}

uint32_t jiffies_to_msecs(uint32_t js)
{
    return (js * SYS_TICK_US / 1000);
}

uint32_t jiffies_to_usecs(uint32_t js)
{
    return (js * SYS_TICK_US);
}

void msleep(uint32_t ms)
{
    uint32_t sleep_ticks = msecs_to_jiffies(ms);
    uint32_t start_tick = jiffies;

    while (jiffies-start_tick < sleep_ticks)
    {
        kthread_yield();
    }
}

void usleep(uint32_t us)
{
    uint32_t sleep_ticks = usecs_to_jiffies(us);
    uint32_t start_tick = jiffies;

    while (jiffies-start_tick < sleep_ticks)
    {
        kthread_yield();
    }
}

void timer_init(void)
{
    TRACE_STR("timer_init()\n");
    jiffies = 0;
    register_handler(0x20, timer_handler);
    timer_set(SYS_TICK_US);
}


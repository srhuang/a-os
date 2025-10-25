#ifndef __KERNEL_INC_TIMER_H
#define __KERNEL_INC_TIMER_H
#include "stdint.h"
#include "math.h"

//=========================
// define
//=========================
#define INPUT_FREQUENCY     (1193180)
#define US_TO_TICKS(us)     DIV_ROUND_UP(us * INPUT_FREQUENCY, 1000000)
#define SYS_TICK_US         (10)

// counter data
#define CONTRER_PORT_0      (0x40)
#define CONTRER_PORT_1      (0x41)
#define CONTRER_PORT_2      (0x42)

// counter control
#define COUNTER_CTL_PORT    (0x43)
#define COUNTER_CH_0        (0x00 << 6)
#define COUNTER_CH_1        (0x01 << 6)
#define COUNTER_CH_2        (0x02 << 6)
#define COUNTER_AC_LATCH    (0x00 << 4)
#define COUNTER_AC_LSB      (0x01 << 4)
#define COUNTER_AC_MSB      (0x02 << 4)
#define COUNTER_AC_ALL      (0x03 << 4)
#define COUNTER_MODE_0      (0x00 << 1)
#define COUNTER_MODE_1      (0x01 << 1)
#define COUNTER_MODE_2      (0x02 << 1)
#define COUNTER_MODE_3      (0x03 << 1)
#define COUNTER_MODE_4      (0x04 << 1)
#define COUNTER_MODE_5      (0x05 << 1)
#define COUNTER_BIN         (0x00 << 0)
#define COUNTER_BCD         (0x01 << 0)

//=========================
// struct
//=========================


//=========================
// external variable
//=========================
extern uint32_t jiffies;

//=========================
// function
//=========================
uint32_t    msecs_to_jiffies(uint32_t);
uint32_t    usecs_to_jiffies(uint32_t);
uint32_t    jiffies_to_msecs(uint32_t);
uint32_t    jiffies_to_usecs(uint32_t);
void        msleep(uint32_t);
void        usleep(uint32_t);
void        timer_init(void);

#endif


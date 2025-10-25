#ifndef __KERNEL_INC_SCHED_H
#define __KERNEL_INC_SCHED_H
#include "thread.h"
#include "timer.h"

//=========================
// define
//=========================
/*
 * default timeslice is 100 msecs
 */
#define SCHED_RR_TIME_SLICE (100 * 1000 / SYS_TICK_US)

//=========================
// struct
//=========================


//=========================
// external variable
//=========================
extern void switch_to(struct task_struct* cur, struct task_struct* next);

//=========================
// function
//=========================
void schedule(void);

#endif

#ifndef __KERNEL_INC_PROCESS_H
#define __KERNEL_INC_PROCESS_H
#include "thread.h"

//=========================
// define
//=========================
#define USR_START_SECTOR    (205)
#define USR_SECTOR_COUNT    (100)
#define USR_INIT_PATH       "/sdb_1/init"
#define PROG_START_SECTOR   (305)
#define PROG_SECTOR_COUNT   (100)

//=========================
// struct
//=========================

//=========================
// external variable
//=========================
extern pid_t pid_init;

//=========================
// function
//=========================
void    tss_init(void);
void    process_init(void);
void    process_switch(struct task_struct* task);
pid_t   sys_fork(void);
int32_t sys_exec(const char* path, char* argv[]);
pid_t   sys_wait(int32_t* status);
void    sys_exit(int32_t exit_code);

#endif

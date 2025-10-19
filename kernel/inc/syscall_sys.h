#ifndef __KERNEL_INC_SYSCALL_SYS_H
#define __KERNEL_INC_SYSCALL_SYS_H

//=========================
// define
//=========================
#define SYSCALL_MAX     (32)

enum SyscallNR {
    SYS_TEST0,
    SYS_TEST1,
    SYS_TEST2,
    SYS_TEST3
};

//=========================
// struct
//=========================


//=========================
// external variable
//=========================

//=========================
// function
//=========================
void syscall_init(void);

#endif

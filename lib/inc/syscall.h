#ifndef __LIB_INC_SYSCALL_H
#define __LIB_INC_SYSCALL_H
//=========================
// define
//=========================

enum SyscallNR {
    // thread.h
    SYS_PS,
    SYS_GETPID,

    // process.h
    SYS_FORK,
    SYS_EXEC,
    SYS_WAIT,
    SYS_EXIT,

    // file.h
    SYS_WRITE,

    SYSCALL_MAX
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

#endif

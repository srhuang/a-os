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
    SYS_OPEN,
    SYS_CLOSE,
    SYS_UNLINK,
    SYS_READ,
    SYS_WRITE,
    SYS_LSEEK,
    SYS_PIPE,
    SYS_DUP2,

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

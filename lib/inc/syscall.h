#ifndef __LIB_INC_SYSCALL_H
#define __LIB_INC_SYSCALL_H
//=========================
// define
//=========================

enum SyscallNR {
    // print.h
    SYS_CLEAR,

    // memory.h
    SYS_MALLOC,
    SYS_FREE,

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
    SYS_STAT,
    SYS_PIPE,
    SYS_DUP2,

    // dir.h
    SYS_OPENDIR,
    SYS_CLOSEDIR,
    SYS_REWINDDIR,
    SYS_READDIR,
    SYS_MKDIR,
    SYS_RMDIR,
    SYS_GETCWD,
    SYS_CHDIR,

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

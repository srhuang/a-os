#include "syscall_sys.h"
#include "syscall.h"
#include "print.h"

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
void* syscall_func[SYSCALL_MAX];

//=========================
// internal functions
//=========================

//=========================
// external functions
//=========================
void syscall_init(void)
{
    TRACE_STR("syscall_init()\n");
#include "print.h"
    syscall_func[SYS_CLEAR]     = cls_screen;
#include "memory.h"
    syscall_func[SYS_MALLOC]    = sys_malloc;
    syscall_func[SYS_FREE]      = sys_free;
#include "thread.h"
    syscall_func[SYS_PS]        = sys_ps;
    syscall_func[SYS_GETPID]    = sys_getpid;
#include "process.h"
    syscall_func[SYS_FORK]      = sys_fork;
    syscall_func[SYS_EXEC]      = sys_exec;
    syscall_func[SYS_WAIT]      = sys_wait;
    syscall_func[SYS_EXIT]      = sys_exit;
#include "file.h"
    syscall_func[SYS_OPEN]      = sys_open;
    syscall_func[SYS_CLOSE]     = sys_close;
    syscall_func[SYS_UNLINK]    = sys_unlink;
    syscall_func[SYS_READ]      = sys_read;
    syscall_func[SYS_WRITE]     = sys_write;
    syscall_func[SYS_LSEEK]     = sys_lseek;
    syscall_func[SYS_STAT]      = sys_stat;
    syscall_func[SYS_PIPE]      = sys_pipe;
    syscall_func[SYS_DUP2]      = sys_dup2;
#include "dir.h"
    syscall_func[SYS_OPENDIR]   = sys_opendir;
    syscall_func[SYS_CLOSEDIR]  = sys_closedir;
    syscall_func[SYS_REWINDDIR] = sys_rewinddir;
    syscall_func[SYS_READDIR]   = sys_readdir;
    syscall_func[SYS_MKDIR]     = sys_mkdir;
    syscall_func[SYS_RMDIR]     = sys_rmdir;
    syscall_func[SYS_GETCWD]    = sys_getcwd;
    syscall_func[SYS_CHDIR]     = sys_chdir;
}


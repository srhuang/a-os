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
    syscall_func[SYS_PIPE]      = sys_pipe;
    syscall_func[SYS_DUP2]      = sys_dup2;
}


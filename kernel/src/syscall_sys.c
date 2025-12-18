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
#include "file.h"
    syscall_func[SYS_WRITE]     = sys_write;
}


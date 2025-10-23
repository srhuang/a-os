#include "syscall_sys.h"
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
void sys_test_syscall0(void)
{
    put_str("sys_test_syscall0()\n");
}

uint32_t sys_test_syscall1(int a)
{
    put_str("sys_test_syscall1()\n");
    put_str("argument 1: 0x");
    put_int(a);
    put_str("\n");
    return 0x9527;
}

uint32_t sys_test_syscall2(int a, int b)
{
    put_str("sys_test_syscall2()\n");
    put_str("argument 1: 0x");
    put_int(a);
    put_str("\n");
    put_str("argument 2: 0x");
    put_int(b);
    put_str("\n");
    return 0x9528;
}

uint32_t sys_test_syscall3(int a, int b, int c)
{
    put_str("sys_test_syscall3()\n");
    put_str("argument 1: 0x");
    put_int(a);
    put_str("\n");
    put_str("argument 2: 0x");
    put_int(b);
    put_str("\n");
    put_str("argument 3: 0x");
    put_int(c);
    put_str("\n");
    return 0x9529;
}

void syscall_init(void)
{
    TRACE_STR("syscall_init()\n");
    syscall_func[SYS_TEST0]      = sys_test_syscall0;
    syscall_func[SYS_TEST1]      = sys_test_syscall1;
    syscall_func[SYS_TEST2]      = sys_test_syscall2;
    syscall_func[SYS_TEST3]      = sys_test_syscall3;
}


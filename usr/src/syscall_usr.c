#include "syscall_usr.h"

//=========================
// external functions
//=========================

void test_syscall0(void)
{
    _syscall0(SYS_TEST0);
}

int test_syscall1(int a)
{
    return _syscall1(SYS_TEST1, a);
}

int test_syscall2(int a, int b)
{
    return _syscall2(SYS_TEST2, a, b);
}

int test_syscall3(int a, int b, int c)
{
    return _syscall3(SYS_TEST3, a, b, c);
}


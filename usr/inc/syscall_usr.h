#ifndef __USR_INC_SYSCALL_USER_H
#define __USR_INC_SYSCALL_USER_H

//=========================
// define
//=========================

// no argument
#define _syscall0(NUMBER) ({\
   int retval;\
   asm volatile (\
   "int $0x80"\
   : "=a" (retval)\
   : "a" (NUMBER)\
   : "memory"\
   );\
   retval;\
})

// 1 argument
#define _syscall1(NUMBER, ARG1) ({\
   int retval;\
   asm volatile (\
   "int $0x80"\
   : "=a" (retval)\
   : "a" (NUMBER), "b" (ARG1)\
   : "memory"\
   );\
   retval;\
})

// 2 arguments
#define _syscall2(NUMBER, ARG1, ARG2) ({\
   int retval;\
   asm volatile (\
   "int $0x80"\
   : "=a" (retval)\
   : "a" (NUMBER), "b" (ARG1), "c" (ARG2)\
   : "memory"\
   );\
   retval;\
})

// 3 arguments
#define _syscall3(NUMBER, ARG1, ARG2, ARG3) ({\
   int retval;\
   asm volatile (\
      "int $0x80"\
      : "=a" (retval)\
      : "a" (NUMBER), "b" (ARG1), "c" (ARG2), "d" (ARG3)\
      : "memory"\
   );\
   retval;\
})

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
void test_syscall0(void);
int test_syscall1(int a);
int test_syscall2(int a, int b);
int test_syscall3(int a, int b, int c);

#endif

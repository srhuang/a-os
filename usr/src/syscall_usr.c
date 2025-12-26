#include "syscall_usr.h"
#include "syscall.h"

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

//=========================
// external functions
//=========================
// thread.h
void ps(void)
{
    _syscall0(SYS_PS);
}
int16_t getpid(void)
{
    return _syscall0(SYS_GETPID);
}

// process.h
int16_t fork(void)
{
    return _syscall0(SYS_FORK);
}

// file.h
int32_t write(uint32_t fd, uint8_t* buf, uint32_t cnt)
{
    return _syscall3(SYS_WRITE, fd, buf, cnt);
}

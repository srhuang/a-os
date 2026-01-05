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
// print.h
void clear(void)
{
    _syscall0(SYS_CLEAR);
}

// memory.h
void* malloc(uint32_t size)
{
    return (void*)_syscall1(SYS_MALLOC, size);
}
void free(void* vaddr)
{
    _syscall1(SYS_FREE, vaddr);
}

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
int32_t exec(const char* path, char* argv[])
{
    return _syscall2(SYS_EXEC, path, argv);
}
int16_t wait(int32_t* status)
{
    return _syscall1(SYS_WAIT, status);
}
void exit(int32_t exit_code)
{
    _syscall1(SYS_EXIT, exit_code);
}

// file.h
int32_t open(const char* path, uint8_t flag)
{
    return _syscall2(SYS_OPEN, path, flag);
}
void close(uint32_t fd)
{
    _syscall1(SYS_CLOSE, fd);
}
int32_t unlink(const char* path)
{
    return _syscall1(SYS_UNLINK, path);
}
int32_t read(uint32_t fd, uint8_t* buf, uint32_t cnt)
{
    return _syscall3(SYS_READ, fd, buf, cnt);
}
int32_t write(uint32_t fd, uint8_t* buf, uint32_t cnt)
{
    return _syscall3(SYS_WRITE, fd, buf, cnt);
}
int32_t lseek(int32_t fd, int32_t offset, uint32_t wh)
{
    return _syscall3(SYS_LSEEK, fd, offset, wh);
}
int32_t stat(const char* path, struct fstat* buf)
{
    return _syscall2(SYS_STAT, path, buf);
}
int32_t pipe(uint32_t fd[2])
{
    return _syscall1(SYS_PIPE, fd);
}
void dup2(uint32_t oldfd, uint32_t newfd)
{
    _syscall2(SYS_DUP2, oldfd, newfd);
}

// dir.h
struct dirstream* opendir(const char* path)
{
    return (struct dirstream*)_syscall1(SYS_OPENDIR, path);
}
void closedir(struct dirstream* dir)
{
    _syscall1(SYS_CLOSEDIR, dir);
}
void rewinddir(struct dirstream* dir)
{
    _syscall1(SYS_REWINDDIR, dir);
}
struct dirent* readdir(struct dirstream* dir)
{
    return (struct dirent*)_syscall1(SYS_READDIR, dir);
}
int32_t mkdir(const char* path)
{
    return _syscall1(SYS_MKDIR, path);
}
int32_t rmdir(const char* path)
{
    return _syscall1(SYS_RMDIR, path);
}
int32_t getcwd(char* buf, uint32_t size)
{
    return _syscall2(SYS_GETCWD, buf, size);
}
int32_t chdir(const char* path)
{
    return _syscall1(SYS_CHDIR, path);
}


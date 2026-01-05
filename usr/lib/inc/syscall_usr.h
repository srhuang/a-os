#ifndef __USR_INC_SYSCALL_USER_H
#define __USR_INC_SYSCALL_USER_H
#include "stdint.h"
#include "fcntl.h"

//=========================
// define
//=========================

//=========================
// struct
//=========================

//=========================
// external variable
//=========================

//=========================
// function
//=========================
//print.h
void    clear(void);

// memory.h
void*   malloc(uint32_t size);
void    free(void* vaddr);

// thread.h
void    ps(void);
int16_t getpid(void);

// process.h
int16_t fork(void);
int32_t exec(const char* path, char* argv[]);
int16_t wait(int32_t* status);
void    exit(int32_t exit_code);

// file.h
int32_t open(const char* path, uint8_t flag);
void    close(uint32_t fd);
int32_t unlink(const char* path);
int32_t read(uint32_t fd, uint8_t* buf, uint32_t cnt);
int32_t write(uint32_t fd, uint8_t* buf, uint32_t cnt);
int32_t lseek(int32_t fd, int32_t offset, uint32_t wh);
int32_t stat(const char* path, struct fstat* buf);
int32_t pipe(uint32_t fd[2]);
void    dup2(uint32_t oldfd, uint32_t newfd);

// dir.h
struct dirstream*   opendir(const char* path);
void                closedir(struct dirstream* dir);
void                rewinddir(struct dirstream* dir);
struct dirent*      readdir(struct dirstream* dir);
int32_t             mkdir(const char* path);
int32_t             rmdir(const char* path);
int32_t             getcwd(char* buf, uint32_t size);
int32_t             chdir(const char* path);

#endif

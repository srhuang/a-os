#ifndef __USR_INC_SYSCALL_USER_H
#define __USR_INC_SYSCALL_USER_H
#include "stdint.h"

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
// thread.h
void    ps(void);
int16_t getpid(void);

// process.h
int16_t fork(void);
int32_t exec(const char* path, char* argv[]);

// file.h
int32_t write(uint32_t fd, uint8_t* buf, uint32_t cnt);

#endif

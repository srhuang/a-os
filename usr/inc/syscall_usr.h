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
// file.h
int32_t write(uint32_t fd, uint8_t* buf, uint32_t cnt);

#endif

#ifndef __USR_INC_PRINTF_H
#define __USR_INC_PRINTF_H
#include "stdint.h"

//=========================
// define
//=========================
#define PRINTF_BUF_SIZE         (1024)

//=========================
// struct
//=========================

//=========================
// external variable
//=========================

//=========================
// function
//=========================
int32_t printf(const char* format, ...);

#endif

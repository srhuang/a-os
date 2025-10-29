#ifndef __LIB_INC_STDIO_H
#define __LIB_INC_STDIO_H
#include "stdint.h"
#include "stddef.h"

//=========================
// define
//=========================
// Variable Argument(va); Argument Pointer(ap)
#define va_start(ap, v)     ap = (va_list)&v
#define va_arg(ap, t)       *((t*)(ap += 4))
#define va_end(ap)          ap = NULL

typedef char* va_list;

//=========================
// function
//=========================
uint32_t vsprintf(char* str, const char* format, char* ap);
uint32_t sprintf(char* buf, const char* format, ...);

#endif


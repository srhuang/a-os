#include "printf.h"
#include "stdio.h"
#include "string.h"
#include "syscall_usr.h"

//=========================
// internal struct
//=========================

//=========================
// global variable
//=========================

//=========================
// internal functions
//=========================

//=========================
// external functions
//=========================
int32_t printf(const char* format, ...)
{
    va_list args;
    uint8_t buf[PRINTF_BUF_SIZE] = {0};

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    return write(stdout_no, buf, strlen(buf));
}

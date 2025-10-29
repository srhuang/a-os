#include "stdio.h"
#include "string.h"

//=========================
// debugging
//=========================
#define DEBUG (1)
#define TRACE_STR(x) do {if(DEBUG) put_str(x);} while(0)
#define TRACE_INT(x) do {if(DEBUG) put_int(x);} while(0)

//=========================
// internal struct
//=========================


//=========================
// global variable
//=========================


//=========================
// internal functions
//=========================
static void itoa(uint32_t value, char** buf_ptr_addr, uint8_t base)
{
    uint32_t m = value % base;
    uint32_t i = value / base;
    if (i) {
        itoa(i, buf_ptr_addr, base);
    }
    if (m < 10) {
        *((*buf_ptr_addr)++) = m + '0';
    } else {
        *((*buf_ptr_addr)++) = m - 10 + 'A';
    }
}

//=========================
// external functions
//=========================
uint32_t vsprintf(char* str, const char* format, va_list ap)
{
    char*   out = str;
    char    c = (*format);

    // argument type
    int32_t arg_int;
    char*   arg_str;

    while(c) {

        // plain text
        if (c != '%') {
            *(out++) = c;
            c = *(++format);
            continue;
        }

        // decode the format
        c = *(++format);
        switch(c)
        {
            case 's': // %s
                arg_str = va_arg(ap, char*);
                strcpy(out, arg_str);
                out += strlen(arg_str);
                break;

            case 'c': // %c
                *(out++) = va_arg(ap, char);
                break;

            case 'd': // %d
                arg_int = va_arg(ap, int);
                if (arg_int < 0) {
                    arg_int = 0 - arg_int; //convert to a positive number
                    *(out++) = '-';
                }
                itoa(arg_int, &out, 10);
                break;

            case 'x': // %x
                arg_int = va_arg(ap, int);
                itoa(arg_int, &out, 16);
                break;

        } // switch

        c = *(++format);
    } // while

    return strlen(out);
}

uint32_t sprintf(char* buf, const char* format, ...)
{
    va_list     args;
    uint32_t    retval;

    va_start(args, format);
    retval = vsprintf(buf, format, args);
    va_end(args);

    return retval;
}


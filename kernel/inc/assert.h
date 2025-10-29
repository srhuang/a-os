#ifndef __KERNEL_INC_ASSERT_H
#define __KERNEL_INC_ASSERT_H

//=========================
// define
//=========================
//#define NDEBUG

#define PANIC(...) kernel_panic(__FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef NDEBUG
    #define assert(CONDITION) ((void)0)
#else
    #define assert(CONDITION) \
        if (CONDITION) {} else {\
            PANIC(#CONDITION);\
        }
#endif

//=========================
// function
//=========================
void kernel_panic(\
    char* filename, int line, const char* func, const char* condition);

#endif

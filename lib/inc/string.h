#ifndef __LIB_INC_STRING_H
#define __LIB_INC_STRING_H
#include "stdint.h"
void        memset(void* str, uint8_t ch, uint32_t n);
void        memcpy(void* dst, const void* src, uint32_t n);
int         memcmp(const void* s1, const void* s2, uint32_t n);
char*       strcpy(char* dst, const char* src);
uint32_t    strlen(const char* str);
int8_t      strcmp (const char* s1, const char* s2); 
char*       strchr(const char* str, const uint8_t ch);
char*       strrchr(const char* str, const uint8_t ch);
char*       strcat(char* dst, const char* src);
uint32_t    strchrs(const char* str, uint8_t ch);
#endif


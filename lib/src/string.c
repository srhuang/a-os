#include "string.h"
#include "stddef.h"

// The memset() function fills the first n bytes of the memory area
// pointed to by str with the constant byte ch
void memset(void* str, uint8_t ch, uint32_t n)
{
    uint8_t* p_str = (uint8_t*)str;
    while (n-- > 0)
        *p_str++ = ch;
}

// The memcpy() function copies n bytes
// from memory area src to memory area dst.
void memcpy(void* dst, const void* src, uint32_t n)
{
    uint8_t* p_dst = dst;
    const uint8_t* p_src = src;
    while (n-- > 0)
        *p_dst++ = *p_src++;
}

// The memcmp() function compares the first n bytes
// of the memory areas s1 and s2
int memcmp(const void* s1, const void* s2, uint32_t n)
{
    const char* p_s1 = s1;
    const char* p_s2 = s2;
    while (n-- > 0)
    {
        if(*p_s1 != *p_s2)
        {
            return *p_s1 > *p_s2 ? 1 : -1;
        }
        p_s1++;
        p_s2++;
    }
    return 0;
}

// The strcpy() function copies the string pointed to by src
// to the buffer pointed to by dst.
char* strcpy(char* dst, const char* src)
{
    char* p_dst = dst;
    while((*dst++ = *src++));
    return p_dst;
}

// The strlen() function calculates the length of the string str.
uint32_t strlen(const char* str)
{
    const char* p_str = str;
    while(*p_str++);
    return (p_str - str - 1);
}

// The strcmp() function compares the two strings s1 and s2.
int8_t strcmp (const char* s1, const char* s2)
{
    while (*s1 != 0 && *s1 == *s2)
    {
        s1++;
        s2++;
    }
    return *s1 < *s2 ? -1 : *s1 > *s2;
}

// The strchr() function returns a pointer to
// the first occurrence of the character ch in the string str.
char* strchr(const char* str, const uint8_t ch)
{
    while (*str != 0)
    {
        if (*str == ch)
        {
            return (char*)str;
        }
        str++;
    }
    return NULL;
}

// The strrchr() function returns a pointer to
// the last occurrence of the character ch in the string str.
char* strrchr(const char* str, const uint8_t ch)
{
    const char* last_char = NULL;
    while (*str != 0)
    {
        if (*str == ch)
        {
            last_char = str;
        }
        str++;
    }
    return (char*)last_char;
}

// The strcat() function appends the src string to the dst string.
// overwriting the terminating null byte ('\0') at the end of dst.
char* strcat(char* dst, const char* src)
{
    char* p_dst = dst;
    while (*p_dst++);
    --p_dst;
    while((*p_dst++ = *src++));
    return dst;
}

// The strchrs() function returns the count of
// occurrence of the character ch in the string str.
uint32_t strchrs(const char* str, uint8_t ch)
{
    uint32_t ch_cnt = 0;
    const char* p = str;
    while(*p != 0)
    {
        if (*p == ch)
        {
            ch_cnt++;
        }
        p++;
    }
    return ch_cnt;
}


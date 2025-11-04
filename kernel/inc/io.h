#ifndef __KERNEL_INC_IO_H
#define __KERNEL_INC_IO_H
#include "stdint.h"

static inline void outb(uint16_t port, uint8_t data)
{
    asm volatile ( "outb %b0, %w1" : : "a" (data), "d" (port));
}

static inline void outsw(uint16_t port, const void* addr, uint32_t cnt)
{
    asm volatile ("cld; rep outsw" \
        : "+S" (addr), "+c" (cnt) \
        : "d" (port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    asm volatile ("inb %w1, %b0" \
        : "=a" (data) \
        : "d" (port));
    return data;
}

static inline void insw(uint16_t port, void* addr, uint32_t cnt)
{
    asm volatile ("cld; rep insw" \
        : "+D" (addr), "+c" (cnt) \
        : "d" (port) \
        : "memory");
}

#endif

#ifndef __KERNEL_INC_IO_H
#define __KERNEL_INC_IO_H
#include "stdint.h"

static inline void outb(uint16_t port, uint8_t data)
{
    asm volatile ( "outb %b0, %w1" : : "a" (data), "Nd" (port));
}

static inline void outsb(uint16_t port, const void* addr, uint32_t cnt)
{
    asm volatile ("cld; rep outsb" \
        : "+S" (addr), "+c" (cnt) \
        : "d" (port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    asm volatile ("inb %w1, %b0" \
        : "=a" (data) \
        : "Nd" (port));
    return data;
}

static inline void insb(uint16_t port, void* addr, uint32_t cnt)
{
    asm volatile ("cld; rep insb" \
        : "+D" (addr), "+c" (cnt) \
        : "d" (port) \
        : "memory");
}

#endif

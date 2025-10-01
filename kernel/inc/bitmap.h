#ifndef __KERNEL_INC_BITMAP_H
#define __KERNEL_INC_BITMAP_H
#include "stdint.h"
#include "stdbool.h"
struct bitmap {
   uint32_t len; // unit is byte
   uint8_t* bits;
};
void    bitmap_init(struct bitmap* btmp, uint8_t* btmp_addr, uint32_t len);
void    bitmap_reset(struct bitmap* btmp);
void    bitmap_set(struct bitmap* btmp, uint32_t btmp_idx, bool value);
bool    bitmap_check(struct bitmap* btmp, uint32_t btmp_idx);
int     bitmap_acquire(struct bitmap* btmp, uint32_t cnt);
void    bitmap_release(struct bitmap* btmp, uint32_t btmp_idx, uint32_t cnt);
#endif


#include "bitmap.h"
#include "string.h"

// The bitmap_init() function initializes the value of bitmap.
void bitmap_init(struct bitmap* btmp, uint8_t* btmp_addr, uint32_t len)
{
    btmp->bits = btmp_addr;
    btmp->len = len;
}

// The bitmap_reset() function resets bits to zero.
void bitmap_reset(struct bitmap* btmp)
{
    memset(btmp->bits, 0, btmp->len);
}

// The bitmap_set() function sets the value(0 or 1) to the bitmap.
void bitmap_set(struct bitmap* btmp, uint32_t btmp_idx, bool value)
{
    uint32_t byte_idx = btmp_idx / 8;
    uint32_t bit_idx = btmp_idx % 8;

    if (true == value) {
        btmp->bits[byte_idx] |= (1 << bit_idx);
    } else {
        btmp->bits[byte_idx] &= ~(1 << bit_idx);
    }
}

// The bitmap_check() function checks the value of bitmap.
bool bitmap_check(struct bitmap* btmp, uint32_t btmp_idx)
{
    uint32_t byte_idx = btmp_idx / 8;
    uint32_t bit_idx = btmp_idx % 8;
    return (btmp->bits[byte_idx] & (1 << bit_idx)) ? true : false;
}

// The bitmap_acquire() function acquires the available contiguous bitmap.
// if successful, then set the bitmap. Otherwise return -1.
int bitmap_acquire(struct bitmap* btmp, uint32_t cnt)
{
    uint32_t byte_idx = 0;

    // search by byte
    while (( 0xff == btmp->bits[byte_idx])
        && (byte_idx < btmp->len))
    {
        byte_idx++;
    }
    // bitmap is full.
    if (byte_idx == btmp->len) {
        return -1;
    }

    // search by bit
    int bit_idx = 0;
    while ((uint8_t)(1 << bit_idx) & btmp->bits[byte_idx])
    {
        bit_idx++;
    }

    // get first available idx
    int free_idx = byte_idx * 8 + bit_idx;
    if (1 == cnt) {
        bitmap_set(btmp, free_idx, true);
        return free_idx;
    }

    // keep searching available contiguous bitmap
    int free_count = 1;
    uint32_t total = btmp->len * 8;
    uint32_t idx = free_idx + 1;
    free_idx = -1; // reset free_idx
    while (idx < total) {
        if (bitmap_check(btmp, idx)) {
            free_count = 0;
        } else {
            free_count++;
        }

        // check if find the n free bitmap
        if (free_count == cnt) {
            free_idx = idx - free_count + 1;
            break;
        }
        idx++;
    } //while

    // set the bitmap if successful
    if(-1 != free_idx) {
        int i;
        for (i=0; i<cnt; i++)
        {
            bitmap_set(btmp, free_idx + i, true);
        }
    }

    return free_idx;
}

// The bitmap_release() function releases contiguous bits
// in the bitmap beginning at btmp_idx.
void bitmap_release(struct bitmap* btmp, uint32_t btmp_idx, uint32_t cnt)
{
    int n = 0;
    while (n < cnt)
    {
        bitmap_set(btmp, btmp_idx + n, false);
        n++;
    }
}


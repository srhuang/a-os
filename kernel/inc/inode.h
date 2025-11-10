#ifndef __KERNEL_INC_INODE_H
#define __KERNEL_INC_INODE_H
#include "fs.h"

//=========================
// define
//=========================
#define DIRECT_IDX_MAX  (12)
#define SINGLE_IDX_MAX  (DIRECT_IDX_MAX + (BLOCK_SIZE / 4))

//=========================
// struct
//=========================

//=========================
// external variable
//=========================

//=========================
// function
//=========================
// Get the partition by inode number
struct ide_ptn*     inode_get_ptn(uint32_t inode_no);

// Read the inode table from the hard disk into memory
struct inode_sys*   inode_open(uint32_t inode_no);

// Remove inode table from memory
void                inode_close(struct inode_sys* inode);

// Acquire a new inode table from the hard disk and store it in memory
struct inode_sys*   inode_acquire(struct ide_ptn* ptn);

// Remove the inode table from both the hard disk and memory
void                inode_release(struct inode_sys* inode);

// Read data from hard disk
int32_t             inode_read(struct inode_sys* inode, \
                        uint32_t pos, uint8_t* dst, uint32_t cnt);

// Write data to hard disk
int32_t             inode_write(struct inode_sys* inode, \
                        uint32_t pos, uint8_t* src, uint32_t cnt);

// Erase the last cnt bytes
int32_t             inode_erase(struct inode_sys* inode, uint32_t cnt);
#endif

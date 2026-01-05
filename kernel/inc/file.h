#ifndef __KERNEL_INC_FILE_H
#define __KERNEL_INC_FILE_H
#include "stdint.h"
#include "fs.h"
#include "fcntl.h"

//=========================
// define
//=========================
#define MAX_FILE_OPEN   (8192)

//=========================
// struct
//=========================
struct file {
    struct inode_sys*   inode;
    uint32_t            pos;
    uint8_t             flag;
};

//=========================
// external variable
//=========================
extern struct file fd_table[MAX_FILE_OPEN];

//=========================
// function
//=========================
int32_t sys_open(const char* path, uint8_t flag);
void    sys_close(uint32_t task_fd_idx);
int32_t sys_unlink(const char* path);
int32_t sys_read(uint32_t task_fd_idx, uint8_t* buf, uint32_t cnt);
int32_t sys_write(uint32_t task_fd_idx, uint8_t* buf, uint32_t cnt);
int32_t sys_lseek(int32_t task_fd_idx, int32_t offset, enum whence wh);
int32_t sys_stat(const char* path, struct fstat* buf);
void    file_init(void);

#endif

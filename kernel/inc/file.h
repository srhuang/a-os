#ifndef __KERNEL_INC_FILE_H
#define __KERNEL_INC_FILE_H
#include "stdint.h"

//=========================
// define
//=========================
#define MAX_FILE_OPEN   (8192)
#define O_RDONLY        (0x1 << 0)
#define O_WRONLY        (0x2 << 0)
#define O_RDWR          (0x3 << 0)
#define O_CREATE        (0x1 << 2)
#define O_TRUNC         (0x1 << 3)
#define O_APPEND        (0x1 << 4)

enum std_fd {
    stdin_no,
    stdout_no,
    stderr_no
};

enum whence {
    SEEK_SET = 1,
    SEEK_CUR,
    SEEK_END
};

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
void    file_init(void);

#endif

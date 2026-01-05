#ifndef __KERNEL_INC_FS_H
#define __KERNEL_INC_FS_H
#include "stdint.h"
#include "lock.h"
#include "list.h"
#include "ide.h"
#include "fcntl.h"

//=========================
// define
//=========================
#define INODE_CNT_MAX   (SECTOR_SIZE * 8)
#define INODE_PER_SEC   (SECTOR_SIZE / sizeof(struct inode_hd))
#define BLOCK_SIZE      (4096)
#define DIR_PER_BLK     (BLOCK_SIZE / sizeof(struct dirent))
#define FS_MAGIC        (0x19890604)
#define ROOT_DIR_MAX    (10)

//=========================
// struct
//=========================
struct inode_hd
{
    uint32_t            i_no;
    uint32_t            i_size;
    uint32_t            i_block[13];
};

struct inode_sys
{
    uint32_t            i_no;
    uint32_t            i_size;
    uint32_t            i_block[13];

    uint32_t            open_cnt;
    struct list_elem    inode_tag;
};

//=========================
// external variable
//=========================
extern struct dirstream root_dir;
extern struct dirent root_blk[ROOT_DIR_MAX];

//=========================
// function
//=========================
void fs_format(struct ide_ptn* ptn);
void fs_mount(struct ide_ptn* ptn);
void fs_unmount(struct ide_ptn* ptn);
void fs_init(void);

#endif

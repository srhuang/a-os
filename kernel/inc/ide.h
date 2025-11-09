#ifndef __KERNEL_INC_IDE_H
#define __KERNEL_INC_IDE_H
#include "stdint.h"
#include "stdbool.h"
#include "list.h"
#include "lock.h"
#include "bitmap.h"

//=========================
// define
//=========================
#define SECTOR_SIZE     (512)

// IDE channel number
#define IDE_CH_NR       (4)

// IDE port offset
#define IDE_DATA_REG    (0x0)
#define IDE_ERROR_REG   (0x1)
#define IDE_SEC_CNT     (0x2)
#define IDE_LBA_LOW     (0x3)
#define IDE_LBA_MID     (0x4)
#define IDE_LBA_HIGH    (0x5)
#define IDE_DRIVE_SET   (0x6)
#define IDE_STA_CMD     (0x7)

// IDE drive set
#define DRIVE_DEFAULT   (0xA0)
#define DRIVE_MASTER    (0x0 << 4)
#define DRIVE_SLAVE     (0x1 << 4)
#define DRIVE_LBA       (0x1 << 6)

// IDE commands
#define CMD_IDENTIFY    (0xEC)
#define CMD_READ_SEC    (0x20)
#define CMD_WRITE_SEC   (0x30)

// IDE status
#define STAT_BSY        (0x01 << 7)
#define STAT_DRQ        (0x01 << 3)

// IDE ID
#define ID_SEC_NUM      (60 * 2) // each element 2-byte

//=========================
// struct
//=========================
struct ide_ptn
{
    char                name[8];
    uint32_t            start_lba;
    uint32_t            sec_cnt;
    struct ide_hd*      hd;
    struct list_elem    ptn_tag;

    // for file system
    uint32_t            inode_base;
    uint32_t            inode_cnt;
    uint32_t            inode_btmp_lba;
    struct bitmap       inode_btmp;
    uint32_t            inode_table_lba;
    uint32_t            blk_btmp_lba;
    struct bitmap       blk_btmp;
    uint32_t            blk_lba;
    struct list         open_inodes;
    struct mutex        mlock;
};

struct ide_hd
{
    char                name[8];
    struct ide_ch*      ch;
    uint8_t             dev_nr;
    bool                valid;
};

struct ide_ch
{
    char                name[8];
    struct ide_hd       dev[2];
    struct mutex        mlock;
    struct semaphore    sema;
    uint16_t            port_base;
    uint8_t             irq;
};

//=========================
// external variable
//=========================
extern struct list ptn_list;

//=========================
// function
//=========================
void ide_read(struct ide_hd* hd, uint32_t lba, void* dst, uint32_t cnt);
void ide_write(struct ide_hd* hd, uint32_t lba, void* src, uint32_t cnt);
void ide_init(void);

#endif

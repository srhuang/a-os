#include "fs.h"
#include "math.h"
#include "printk.h"
#include "memory.h"
#include "string.h"
#include "assert.h"

//=========================
// debugging
//=========================
#define DEBUG

#ifdef DEBUG
    #define pr_debug(fmt, ...) printk(fmt, ##__VA_ARGS__)
#else
    #define pr_debug(fmt, ...) do { } while (0)
#endif

//=========================
// internal struct
//=========================
struct super_block
{
    uint32_t magic;

    // inode
    uint32_t inode_cnt;
    uint32_t inode_btmp_lba;
    uint32_t inode_btmp_sec;
    uint32_t inode_table_lba;
    uint32_t inode_table_sec;

    // data block
    uint32_t blk_cnt;
    uint32_t blk_btmp_lba;
    uint32_t blk_btmp_sec;
    uint32_t blk_lba;

    uint8_t  pad[472]; // 1 sector = 512 bytes
} __attribute__ ((packed)); // avoid data alignment

//=========================
// global variable
//=========================
struct dirstream root_dir;
struct dirent root_blk[ROOT_DIR_MAX];

//=========================
// internal functions
//=========================
static void root_dir_init()
{
    struct inode_sys* root_inode = \
        (struct inode_sys*)sys_malloc(sizeof(struct inode_sys));

    struct dirent* dir = (struct dirent*)root_blk;
    memset(dir, 0, BLOCK_SIZE);
    // init '.'
    memcpy(dir->filename, ".", 1);
    dir->i_no = 0;
    dir->f_type = FT_DIR;
    dir++;

    // init '..'
    memcpy(dir->filename, "..", 2);
    dir->i_no = 0;
    dir->f_type = FT_DIR;

    // init root inode
    root_inode->i_no = 0;
    root_inode->i_size = sizeof(struct dirent) * 2;
    root_inode->i_block[0] = (uint32_t)root_blk;
    root_inode->open_cnt = 0;

    // init root dir
    root_dir.inode = root_inode;
    root_dir.pos = 0;
}

//=========================
// external functions
//=========================
void fs_format(struct ide_ptn* ptn)
{
    struct super_block sb;

    // init super block
    sb.magic = FS_MAGIC;
    // init super block: inode
    sb.inode_cnt        = INODE_CNT_MAX;
    sb.inode_btmp_lba   = ptn->start_lba + 2;
    sb.inode_btmp_sec   = 1;
    sb.inode_table_lba  = sb.inode_btmp_lba + sb.inode_btmp_sec;
    sb.inode_table_sec  = INODE_CNT_MAX / INODE_PER_SEC;
    // init super block: data block
    uint32_t used_sec = 2 + sb.inode_btmp_sec + sb.inode_table_sec;
    uint32_t free_sec = ptn->sec_cnt - used_sec;
    sb.blk_cnt          = free_sec * SECTOR_SIZE / BLOCK_SIZE;
    sb.blk_btmp_lba     = sb.inode_table_lba + sb.inode_table_sec;
    sb.blk_btmp_sec     = DIV_ROUND_UP(sb.blk_cnt, SECTOR_SIZE * 8);
    // init super block: data block must exclude bitmap for block
    sb.blk_cnt          = \
        (free_sec - sb.blk_btmp_sec) * SECTOR_SIZE / BLOCK_SIZE;
    // init super block: data block
    sb.blk_lba   = sb.blk_btmp_lba + sb.blk_btmp_sec;

    pr_debug("%s format: ", ptn->name);
    pr_debug("inode_table_sec:%d ",sb.inode_table_sec);
    pr_debug("blk_cnt:%d, ", sb.blk_cnt);
    pr_debug("blk_lba:%d ", sb.blk_lba);
    pr_debug("\n");

    // write super block to hd
    ide_write(ptn->hd, ptn->start_lba + 1, &sb, 1);

    // The block bitmap is larger than the inode bitmap
    uint32_t buf_size = sb.blk_btmp_sec * SECTOR_SIZE;
    uint8_t* buf = (uint8_t*)sys_malloc(buf_size);

    // reset inode bitmap
    memset(buf, 0, buf_size);
    // for root dir
    buf[0] = 0x01;
    ide_write(ptn->hd, sb.inode_btmp_lba, buf, sb.inode_btmp_sec);

    // set inode table for root dir
    memset(buf, 0, buf_size);
    struct inode_hd* root = (struct inode_hd*)buf;
    root->i_no = 0;
    root->i_size = sizeof(struct dirent) * 2;
    root->i_block[0] = sb.blk_lba;
    ide_write(ptn->hd, sb.inode_table_lba, buf, 1);

    // reset block bitmap
    memset(buf, 0, buf_size);
    //for root dir
    buf[0] = 0x01;
    // align the block bitmap with block count
    uint32_t last_byte = sb.blk_cnt / 8;
    uint32_t last_bit  = sb.blk_cnt % 8;
    uint32_t last_size = SECTOR_SIZE - (last_byte % SECTOR_SIZE);
    memset(&buf[last_byte], 0xFF, last_size);
    int idx=0;
    while (idx <= last_bit)
    {
        buf[last_byte] &= ~(1 << idx);
        idx++;
    }
    ide_write(ptn->hd, sb.blk_btmp_lba, buf, sb.blk_btmp_sec);

    // set root dir data block
    memset(buf, 0, buf_size);
    struct dirent* dir = (struct dirent*)buf;

    // init '.'
    memcpy(dir->filename, ".", 1);
    dir->i_no = 0;
    dir->f_type = FT_DIR;
    dir++;

    // init '..'
    memcpy(dir->filename, "..", 2);
    dir->i_no = 0;
    dir->f_type = FT_DIR;

    // write to hd
    ide_write(ptn->hd, sb.blk_lba, buf, 1);

    sys_free(buf);
}

void fs_mount(struct ide_ptn* ptn)
{
    struct super_block* sb;
    static uint32_t inode_max = 0;

    //pr_debug("%s +++\n", __func__);

    // init: get super block
    sb = (struct super_block*)sys_malloc(sizeof(struct super_block));
    ide_read(ptn->hd, ptn->start_lba + 1, sb, 1);
    // init: set inode
    ptn->inode_cnt = sb->inode_cnt;
    ptn->inode_btmp_lba = sb->inode_btmp_lba;
    ptn->inode_btmp.len = sb->inode_btmp_sec * SECTOR_SIZE;
    ptn->inode_btmp.bits = \
        (uint8_t*)sys_malloc(ptn->inode_btmp.len);
    ide_read(ptn->hd, ptn->inode_btmp_lba, \
        ptn->inode_btmp.bits, sb->inode_btmp_sec);
    ptn->inode_table_lba = sb->inode_table_lba;
    // init: set block
    ptn->blk_btmp_lba = sb->blk_btmp_lba;
    ptn->blk_btmp.len = sb->blk_btmp_sec * SECTOR_SIZE;
    ptn->blk_btmp.bits = \
        (uint8_t*)sys_malloc(ptn->blk_btmp.len);
    ide_read(ptn->hd, ptn->blk_btmp_lba, \
        ptn->blk_btmp.bits, sb->blk_btmp_sec);
    ptn->blk_lba = sb->blk_lba;
    // init: list
    list_init(&ptn->open_inodes);
    // init: lock
    mutex_init(&ptn->mlock);
    // init: set inode base
    if (0 == ptn->inode_base) {
        ptn->inode_base = inode_max + 1;
        inode_max += sb->inode_cnt;
    }
    pr_debug("%s mount:inode base=%d, ", ptn->name, ptn->inode_base);

    // add to root dir
    struct dirent* dir = (struct dirent*)root_dir.inode->i_block[0];
    //max idx = 4096 / 24 = 170 (round down)
    int dir_idx=0;
    while (dir_idx < ROOT_DIR_MAX)
    {
        if (FT_UNKNOWN == (dir + dir_idx)->f_type) {
            strcpy((dir + dir_idx)->filename, ptn->name);
            (dir + dir_idx)->i_no = ptn->inode_base;
            (dir + dir_idx)->f_type = FT_DIR;
            root_dir.inode->i_size += sizeof(struct dirent);
            break;
        }
        dir_idx++;
    } //while
    // check result
    assert(dir_idx < ROOT_DIR_MAX);
    pr_debug("root dir size: %d\n", root_dir.inode->i_size);

    sys_free(sb);
    //pr_debug("%s ---\n", __func__);
}

void fs_unmount(struct ide_ptn* ptn)
{
    // remove from root dir
    struct dirent* dir = (struct dirent*)root_dir.inode->i_block[0];
    //max idx = 4096 / 24 = 170 (round down)
    int dir_idx=0;
    while (dir_idx < ROOT_DIR_MAX)
    {
        if (!strcmp((dir + dir_idx)->filename, ptn->name)) {
            (dir + dir_idx)->f_type = FT_UNKNOWN;
            root_dir.inode->i_size -= sizeof(struct dirent);
            pr_debug("%s unmount:root dir size: %d\n", \
                (dir + dir_idx)->filename, root_dir.inode->i_size);
            break;
        }
        dir_idx++;
    } // while
}

void fs_init()
{
    struct ide_ptn* ptn;
    struct list_elem* cur_elem;

    pr_debug("%s +++\n", __func__);

    // init root dir
    root_dir_init();

    // check empty
    if (list_empty(&ptn_list)) {
        pr_debug("There is no partition\n");
        return;
    }

    struct super_block* sb_buf = \
        (struct super_block*)sys_malloc(SECTOR_SIZE);

    // parsing each partition
    cur_elem = ptn_list.head.next;
    while (cur_elem != &ptn_list.tail)
    {
        ptn = elem2entry(struct ide_ptn, ptn_tag, cur_elem);
        //pr_debug("partition: %s\n", ptn->name);

        // get the super block
        ide_read(ptn->hd, ptn->start_lba + 1, sb_buf, 1);

        // check file system
        if (FS_MAGIC != sb_buf -> magic) {
            fs_format(ptn);
        }

        // mount each partition
        fs_mount(ptn);

        // next partition
        cur_elem = cur_elem->next;
    } // while
    sys_free(sb_buf);

    pr_debug("%s ---\n", __func__);
}


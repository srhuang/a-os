#include "inode.h"
#include "memory.h"
#include "stddef.h"
#include "string.h"
#include "bitmap.h"
#include "assert.h"
#include "math.h"
#include "printk.h"

//=========================
// debugging
//=========================
//#define DEBUG

#ifdef DEBUG
    #define pr_debug(fmt, ...) printk(fmt, ##__VA_ARGS__)
#else
    #define pr_debug(fmt, ...) do { } while (0)
#endif

//=========================
// internal struct
//=========================

//=========================
// global variable
//=========================

//=========================
// internal functions
//=========================
static int32_t blk_acquire(struct ide_ptn* ptn)
{
    // acquire free block bitmap
    int32_t blk_btmp_idx = bitmap_acquire(&ptn->blk_btmp, 1);
    if (-1 == blk_btmp_idx) {
        return -1;
    }

    // update block bitmap to hard disk
    uint32_t blk_btmp_sec = blk_btmp_idx / (SECTOR_SIZE * 8);
    uint32_t blk_btmp_offset = blk_btmp_sec * SECTOR_SIZE;
    uint8_t* blk_btmp_addr = ptn->blk_btmp.bits + blk_btmp_offset;
    uint32_t blk_btmp_lba = ptn->blk_btmp_lba + blk_btmp_sec;
    ide_write(ptn->hd, blk_btmp_lba, blk_btmp_addr, 1);

    // clear block on hard disk
    char* buf = (char*)sys_malloc(BLOCK_SIZE);
    memset(buf, 0, BLOCK_SIZE);
    uint32_t blk_sec = (blk_btmp_idx * BLOCK_SIZE) / SECTOR_SIZE;
    uint32_t blk_lba = ptn->blk_lba + blk_sec;
    ide_write(ptn->hd, blk_lba, buf, (BLOCK_SIZE / SECTOR_SIZE));
    sys_free(buf);

    pr_debug("%s:blk_lba(%d).\n", __func__, blk_lba);

    return blk_lba;
}

static void blk_release(struct ide_ptn* ptn, uint32_t blk_lba)
{
    pr_debug("%s:blk_lba(%d).\n", __func__, blk_lba);

    // release block bitmap
    uint32_t blk_btmp_idx = \
        (blk_lba - ptn->blk_lba) * SECTOR_SIZE / BLOCK_SIZE;
    bitmap_release(&ptn->blk_btmp, blk_btmp_idx, 1);

    // update block bitmap to hard disk
    uint32_t blk_btmp_sec = blk_btmp_idx / (SECTOR_SIZE * 8);
    uint32_t blk_btmp_offset = blk_btmp_sec * SECTOR_SIZE;
    uint8_t* blk_btmp_addr = ptn->blk_btmp.bits + blk_btmp_offset;
    uint32_t blk_btmp_lba = ptn->blk_btmp_lba + blk_btmp_sec;
    ide_write(ptn->hd, blk_btmp_lba, blk_btmp_addr, 1);
}

static int32_t blk_read( \
    struct inode_sys* inode, uint32_t blk_idx, uint8_t* dst, uint32_t cnt)
{
    uint32_t blk_lba;
    uint32_t* buf = NULL;
    int32_t ret = 0;

    // get partition
    struct ide_ptn* ptn = inode_get_ptn(inode->i_no);
    assert(NULL != ptn);

    // check cnt
    if (0 == cnt) {
        printk("%s:cnt MUST NOT be zero.\n", __func__);
        return -1;
    }

    // get max block index
    uint32_t blk_idx_max = inode->i_size / BLOCK_SIZE;

    // check blk_idx
    if ((blk_idx > blk_idx_max) || (blk_idx >= SINGLE_IDX_MAX)) {
        printk("%s:blk_idx(%d) is invalid.\n", __func__, blk_idx);
        return -1;
    }

    // check cnt
    if (((blk_idx + cnt - 1) > blk_idx_max) \
        || ((blk_idx + cnt - 1) >= SINGLE_IDX_MAX)) 
    {
        printk("%s:cnt(%d) is invalid.\n", __func__, cnt);
        return -1;
    }

    // get the single indirect blocks
    if ((blk_idx_max >= DIRECT_IDX_MAX) \
        && (0 != inode->i_block[DIRECT_IDX_MAX]))
    {
        blk_lba = inode->i_block[DIRECT_IDX_MAX];
        if (0 == blk_lba) {
            printk("%s:There is NO single indirect blocks.\n", __func__);
            return -1;
        }
        uint32_t* buf = (uint32_t*)sys_malloc(BLOCK_SIZE);
        ide_read(ptn->hd, blk_lba, buf, (BLOCK_SIZE / SECTOR_SIZE));
    }

    // read the data from hard disk
    uint32_t idx = blk_idx;
    while (idx < (blk_idx + cnt))
    {
        // get LBA
        if (idx < DIRECT_IDX_MAX) {
            // direct blocks
            blk_lba = inode->i_block[idx];
        } else if ((DIRECT_IDX_MAX <= idx) \
            && (idx < SINGLE_IDX_MAX)) {
            // single blocks
            blk_lba = *(buf + (idx - DIRECT_IDX_MAX));
        } else {
            printk("%s:Out of bounds for idx(%d).\n", __func__, idx);
            ret = -1;
            break;
        }
        if (0 == blk_lba) {
            printk("%s:Invalid LBA for idx(%d).\n", __func__, idx);
            ret = -1;
            break;
        }
        // read from hard disk
        ide_read(ptn->hd, blk_lba, dst, (BLOCK_SIZE / SECTOR_SIZE));
        dst += BLOCK_SIZE;
        ret += BLOCK_SIZE;
        idx++;
    }

    // release memory
    if (NULL != buf) {
        sys_free(buf);
    }
    return ret;
}

static int32_t blk_write( \
    struct inode_sys* inode, uint32_t blk_idx, uint8_t* src, uint32_t cnt)
{
    uint32_t blk_lba;
    uint32_t* buf = NULL;

    // get partition
    struct ide_ptn* ptn = inode_get_ptn(inode->i_no);
    assert(NULL != ptn);

    // check cnt
    if (0 == cnt) {
        printk("%s:cnt MUST NOT be zero.\n", __func__);
        return -1;
    }

    // check max block index
    uint32_t blk_idx_max = blk_idx + cnt;
    if (((blk_idx_max) >= SINGLE_IDX_MAX)) {
        printk("%s:blk_idx(%d) + cnt(%d) is out of bounds.\n", \
            __func__, blk_idx, cnt);
        return -1;
    }

    // get the single indirect blocks
    if ((blk_idx_max >= DIRECT_IDX_MAX)) {
        buf = (uint32_t*)sys_malloc(BLOCK_SIZE);
        memset(buf, 0, BLOCK_SIZE);
        if (0 != inode->i_block[DIRECT_IDX_MAX]) {
            // already acquired
            blk_lba = inode->i_block[DIRECT_IDX_MAX];
            ide_read(ptn->hd, blk_lba, buf, (BLOCK_SIZE / SECTOR_SIZE));
        } else {
            // need acquire new block
            blk_lba = blk_acquire(ptn);
            inode->i_block[DIRECT_IDX_MAX] = blk_lba;
        }
    }

    // wirte the data to hard disk
    uint32_t idx = blk_idx;
    while (idx < (blk_idx + cnt))
    {
        // get LBA
        if (idx < DIRECT_IDX_MAX) {
            // direct blocks
            blk_lba = inode->i_block[idx];
            if (0 == blk_lba) {
                blk_lba = blk_acquire(ptn);
                inode->i_block[idx] = blk_lba;
            }
        } else if ((DIRECT_IDX_MAX <= idx) \
            && (idx < SINGLE_IDX_MAX)) {
            // single blocks
            blk_lba = *(buf + (idx - DIRECT_IDX_MAX));
            if (0 == blk_lba) {
                blk_lba = blk_acquire(ptn);
                *(buf + (idx - DIRECT_IDX_MAX)) = blk_lba;
            }
        } else {
            printk("%s:Out of bounds for idx(%d).\n", __func__, idx);
            break;
        }

        // write to hard disk
        ide_write(ptn->hd, blk_lba, src, (BLOCK_SIZE / SECTOR_SIZE));
        src += BLOCK_SIZE;
        idx++;
    }

    // update single indirect index
    if (blk_idx_max >= DIRECT_IDX_MAX) {
        blk_lba = inode->i_block[DIRECT_IDX_MAX];
        ide_write(ptn->hd, blk_lba, buf, 1);
    }

    // release memory
    if (NULL != buf) {
        sys_free(buf);
    }
    return 0;
}

static void inode_table_update(struct inode_sys* inode)
{
    // get partition
    struct ide_ptn* ptn = inode_get_ptn(inode->i_no);

    // transfer inode number
    uint32_t inode_no_hd = inode->i_no - ptn->inode_base;

    // update inode table to hard disk
    char* buf = (char*)sys_malloc(SECTOR_SIZE);
    uint32_t inode_sec = inode_no_hd / INODE_PER_SEC;
    uint32_t inode_offset = \
        (inode_no_hd % INODE_PER_SEC) * sizeof(struct inode_hd);
    uint32_t inode_lba = ptn->inode_table_lba + inode_sec;

    pr_debug("inode_sec=%d, inode_offset=%d, inode_lba=%d\n", \
        inode_sec, inode_offset, inode_lba);

    ide_read(ptn->hd, inode_lba, buf, 1);
    memcpy(buf + inode_offset, inode, sizeof(struct inode_hd));
    ide_write(ptn->hd, inode_lba, buf, 1);
    sys_free(buf);
}

//=========================
// external functions
//=========================
struct ide_ptn* inode_get_ptn(uint32_t inode_no)
{
    struct ide_ptn* ptn;
    struct list_elem* cur_elem;

    // parsing each partition
    cur_elem = ptn_list.head.next;
    while (cur_elem != &ptn_list.tail)
    {
        ptn = elem2entry(struct ide_ptn, ptn_tag, cur_elem);

        // check inode number
        uint32_t inode_max = ptn->inode_base + ptn->inode_cnt - 1;
        if ((inode_no >= ptn->inode_base) && (inode_no<= inode_max))
        {
            return ptn;
        }

        // next partition
        cur_elem = cur_elem->next;
    }

    return NULL;
}

struct inode_sys* inode_open(uint32_t inode_no)
{
    struct list_elem* elem;
    struct inode_sys* ret_inode = NULL;
    struct inode_sys* inode;

    // get partition
    struct ide_ptn* ptn = inode_get_ptn(inode_no);
    assert(NULL != ptn);

    mutex_lock(&ptn->mlock);

    // get inode from cache
    elem = ptn->open_inodes.head.next;
    while (elem != &ptn->open_inodes.tail)
    {
        inode = elem2entry(struct inode_sys, inode_tag, elem);
        if (inode->i_no == inode_no) {
            inode->open_cnt++;
            ret_inode = inode;
            break;
        }
        elem = elem->next;
    }

    if (NULL != ret_inode){
        mutex_unlock(&ptn->mlock);
        return ret_inode;
    }

    // transfer the inode number
    uint32_t inode_no_hd = inode_no - ptn->inode_base;

    // check inode valid
    if (!bitmap_check(&ptn->inode_btmp, inode_no_hd)) {
        printk("%s:Invalid inode(%d).\n", __func__, inode_no);
        mutex_unlock(&ptn->mlock);
        return NULL;
    }

    // get inode from hard disk
    uint8_t* buf = (uint8_t*)sys_malloc(SECTOR_SIZE);
    ret_inode = (struct inode_sys*)sys_malloc(sizeof(struct inode_sys));
    uint32_t inode_sec = inode_no_hd / INODE_PER_SEC;
    uint32_t inode_offset = \
        (inode_no_hd % INODE_PER_SEC) * sizeof(struct inode_hd);
    uint32_t inode_lba = ptn->inode_table_lba + inode_sec;
    ide_read(ptn->hd, inode_lba, buf, 1);
    memcpy(ret_inode, buf + inode_offset, sizeof(struct inode_hd));
    sys_free(buf);

    // transfer the inode number and add it to the cache
    ret_inode->i_no = ptn->inode_base + inode_no_hd;
    ret_inode->open_cnt = 1;
    list_push(&ptn->open_inodes, &ret_inode->inode_tag);

    mutex_unlock(&ptn->mlock);
    return ret_inode;
}

void inode_close(struct inode_sys* inode)
{
    // get partition
    struct ide_ptn* ptn = inode_get_ptn(inode->i_no);
    assert(NULL != ptn);

    mutex_lock(&ptn->mlock);

    inode->open_cnt--;
    if(0 == inode->open_cnt) {
        list_remove(&inode->inode_tag);
        sys_free(inode);
    }

    mutex_unlock(&ptn->mlock);
}

struct inode_sys* inode_acquire(struct ide_ptn* ptn)
{
    mutex_lock(&ptn->mlock);

    // acquire free inode bitmap in memory
    int32_t inode_btmp_idx = bitmap_acquire(&ptn->inode_btmp, 1);
    if (-1 == inode_btmp_idx) {
        mutex_unlock(&ptn->mlock);
        return NULL;
    }

    // update inode bitmap to hard disk
    uint32_t inode_btmp_sec = inode_btmp_idx / (SECTOR_SIZE * 8);
    uint32_t inode_btmp_offset = inode_btmp_sec * SECTOR_SIZE;
    uint8_t* inode_btmp_addr = ptn->inode_btmp.bits + inode_btmp_offset;
    uint32_t inode_btmp_lba = ptn->inode_btmp_lba + inode_btmp_sec;
    ide_write(ptn->hd, inode_btmp_lba, inode_btmp_addr, 1);

    // allocate inode table
    struct inode_sys* inode = \
        (struct inode_sys*)sys_malloc(sizeof(struct inode_sys));
    inode->i_no = inode_btmp_idx;
    inode->i_size = 0;
    inode->open_cnt = 0;
    for (int idx=0; idx<13; idx++)
    {
        inode->i_block[idx] = 0;
    }

    // update inode table to hard disk
    char* buf = (char*)sys_malloc(SECTOR_SIZE);
    uint32_t inode_sec = inode->i_no / INODE_PER_SEC;
    uint32_t inode_offset = \
        (inode->i_no % INODE_PER_SEC) * sizeof(struct inode_hd);
    uint32_t inode_lba = ptn->inode_table_lba + inode_sec;

    ide_read(ptn->hd, inode_lba, buf, 1);
    memcpy(buf + inode_offset, inode, sizeof(struct inode_hd));
    ide_write(ptn->hd, inode_lba, buf, 1);
    sys_free(buf);

    // transfer the inode number and add it to the cache
    inode->i_no += ptn->inode_base;
    inode->open_cnt = 1;
    list_push(&ptn->open_inodes, &inode->inode_tag);

    mutex_unlock(&ptn->mlock);
    return inode;
}

void inode_release(struct inode_sys* inode)
{
    // get partition
    struct ide_ptn* ptn = inode_get_ptn(inode->i_no);
    assert(NULL != ptn);

    mutex_lock(&ptn->mlock);

    // release blocks belonging to the inode
    uint32_t blk_lba;
    // for direct blocks
    for (int idx=0; idx<DIRECT_IDX_MAX; idx++)
    {
        blk_lba = inode->i_block[idx];
        if (0 != blk_lba) {
            blk_release(ptn, blk_lba);
            inode->i_block[idx] = 0;
        }
    }
    // for single indirect blocks
    blk_lba = inode->i_block[DIRECT_IDX_MAX];
    if (0 != blk_lba) {
        uint32_t* buf = (uint32_t*)sys_malloc(BLOCK_SIZE);
        uint32_t buf_idx = 0;
        uint32_t buf_idx_max = BLOCK_SIZE / sizeof(uint32_t);
        ide_read(ptn->hd, blk_lba, buf, (BLOCK_SIZE / SECTOR_SIZE));
        while (buf_idx < buf_idx_max)
        {
            if (0 != *buf) {
                blk_release(ptn, *buf);
            }
            buf++;
            buf_idx++;
        }
        blk_release(ptn, blk_lba);
        inode->i_block[DIRECT_IDX_MAX] = 0;
        sys_free(buf);
    }

    // transfer inode number
    uint32_t i_no_hd = inode->i_no - ptn->inode_base;

    // release inode bitmap
    bitmap_release(&ptn->inode_btmp, i_no_hd, 1);

    // update inode bitmap to hard disk
    uint32_t inode_btmp_sec = i_no_hd / (SECTOR_SIZE * 8);
    uint32_t inode_btmp_offset = inode_btmp_sec * SECTOR_SIZE;
    uint8_t* inode_btmp_addr = ptn->inode_btmp.bits + inode_btmp_offset;
    uint32_t inode_btmp_lba = ptn->inode_btmp_lba + inode_btmp_sec;
    ide_write(ptn->hd, inode_btmp_lba, inode_btmp_addr, 1);

    // remove from cache
    list_remove(&inode->inode_tag);
    sys_free(inode);

    mutex_unlock(&ptn->mlock);
}

int32_t inode_read( \
    struct inode_sys* inode, uint32_t pos, uint8_t* dst, uint32_t cnt)
{
    int32_t ret = -1;

    // get partition
    struct ide_ptn* ptn = inode_get_ptn(inode->i_no);
    assert(NULL != ptn);

    mutex_lock(&ptn->mlock);

    uint32_t blk_head_idx = pos / BLOCK_SIZE;
    uint32_t blk_head_offset = pos % BLOCK_SIZE;
    uint32_t blk_tail_idx = (pos + cnt - 1) / BLOCK_SIZE ;
    uint32_t blk_tail_offset = (pos + cnt - 1) % BLOCK_SIZE;
    uint32_t blk_cnt = blk_tail_idx - blk_head_idx + 1;
    uint8_t* buf = (uint8_t*)sys_malloc(BLOCK_SIZE);

    // check size is valid
    if (( pos + cnt) > inode->i_size) {
        printk("%s:Over the size of inode.(%d)\n", __func__, inode->i_size);
        goto out;
    }

    // handle the first block
    if (-1 == blk_read(inode, blk_head_idx, buf, 1)) {
        goto out;
    }
    if (1 == blk_cnt) {
        memcpy(dst, buf + blk_head_offset, cnt);
    } else {
        memcpy(dst, buf + blk_head_offset, BLOCK_SIZE - blk_head_offset);
        dst += BLOCK_SIZE - blk_head_offset;
    }

    // handle the middle block
    if (blk_cnt > 2) {
        if (-1 == blk_read(inode, blk_head_idx + 1, dst, blk_cnt - 2)) {
            goto out;
        }
        dst += (blk_cnt - 2) * BLOCK_SIZE;
    }

    // handle the last block
    if (blk_cnt > 1) {
        if (-1 == blk_read(inode, blk_tail_idx, buf, 1)) {
            goto out;
        }
        memcpy(dst, buf, blk_tail_offset + 1);
    }

    // success
    ret = 0;

out:
    sys_free(buf);
    mutex_unlock(&ptn->mlock);
    return ret;
}

int32_t inode_write( \
    struct inode_sys* inode, uint32_t pos, uint8_t* src, uint32_t cnt)
{
    int32_t ret = -1;

    // get partition
    struct ide_ptn* ptn = inode_get_ptn(inode->i_no);
    assert(NULL != ptn);

    mutex_lock(&ptn->mlock);

    uint32_t blk_head_idx = pos / BLOCK_SIZE;
    uint32_t blk_head_offset = pos % BLOCK_SIZE;
    uint32_t blk_tail_idx = (pos + cnt - 1) / BLOCK_SIZE;
    uint32_t blk_tail_offset = (pos + cnt - 1) % BLOCK_SIZE;
    uint32_t blk_cnt = blk_tail_idx - blk_head_idx + 1;
    uint8_t* buf = (uint8_t*)sys_malloc(BLOCK_SIZE);
    memset(buf, 0, BLOCK_SIZE);

    // handle the first block
    if (0 != inode->i_block[blk_head_idx]) {
        ret = blk_read(inode, blk_head_idx, buf, 1);
        if (-1 == ret) {
            goto out;
        }
    }
    if (1 == blk_cnt) {
        memcpy(buf + blk_head_offset, src, cnt);
    } else {
        memcpy(buf + blk_head_offset, src, BLOCK_SIZE - blk_head_offset);
    }
    ret = blk_write(inode, blk_head_idx, buf, 1);
    if (-1 == ret) {
        goto out;
    }
    src += BLOCK_SIZE - blk_head_offset;

    // handle the middle block
    if (blk_cnt > 2) {
        ret = blk_write(inode, blk_head_idx + 1, src, blk_cnt - 2);
        if (-1 == ret) {
            goto out;
        }
        src += (blk_cnt - 2) * BLOCK_SIZE;
    }

    // handle the last block
    if (blk_cnt > 1) {
        if (0 != inode->i_block[blk_tail_idx]) {
            ret = blk_read(inode, blk_tail_idx, buf, 1);
            if (-1 == ret) {
                goto out;
            }
        }
        memcpy(buf, src, blk_tail_offset + 1);
        if (-1 == blk_write(inode, blk_tail_idx, buf, 1)) {
            goto out;
        }
    }

    // update the size
    if ((pos + cnt) > inode->i_size) {
        inode->i_size = pos + cnt;
        inode_table_update(inode);
    }

    // success
    ret = 0;

out:
    sys_free(buf);
    mutex_unlock(&ptn->mlock);
    return ret;
}

int32_t inode_erase(struct inode_sys* inode, uint32_t cnt)
{
    int32_t ret = -1;
    uint32_t* buf = NULL;

    // get partition
    struct ide_ptn* ptn = inode_get_ptn(inode->i_no);
    assert(NULL != ptn);

    mutex_lock(&ptn->mlock);

    uint32_t blk_idx_before = inode->i_size / BLOCK_SIZE;
    uint32_t blk_idx_after = (inode->i_size - cnt) / BLOCK_SIZE;
    uint32_t blk_erase = blk_idx_after - blk_idx_before;

    // release block
    if (0 != blk_erase) {
        uint32_t idx;
        uint32_t blk_lba;
        // get the single indirect blocks
        if (blk_idx_before >= DIRECT_IDX_MAX) {
            buf = (uint32_t*)sys_malloc(BLOCK_SIZE);
            memset(buf, 0, BLOCK_SIZE);
            blk_lba = inode->i_block[DIRECT_IDX_MAX];
            ide_read(ptn->hd, blk_lba, buf, (BLOCK_SIZE / SECTOR_SIZE));
        }

        // release each block
        for(idx = blk_idx_before; idx > blk_idx_after; idx--)
        {
            if (idx < DIRECT_IDX_MAX) {
                blk_lba = inode->i_block[idx];
                blk_release(ptn, blk_lba);
                inode->i_block[idx] = 0;
            } else if ((DIRECT_IDX_MAX <= idx) \
                && (idx < SINGLE_IDX_MAX)) {
                // single indirect blocks
                blk_lba = *(buf + (idx - DIRECT_IDX_MAX));
                blk_release(ptn, blk_lba);
                *(buf + (idx - DIRECT_IDX_MAX)) = 0;
            } else {
                printk("%s:Out of bounds for idx(%d).\n", __func__, idx);
                goto out;
                break;
            }
        } // for

        // update the single indirect blocks
        if (blk_idx_before >= DIRECT_IDX_MAX) {
            blk_lba = inode->i_block[DIRECT_IDX_MAX];
            if (blk_idx_after < DIRECT_IDX_MAX) {
                blk_release(ptn, blk_lba);
            } else {
                ide_write(ptn->hd, blk_lba, buf, (BLOCK_SIZE / SECTOR_SIZE));
            }
        }
    } // if (0 != blk_erase)

    // update the size
    inode->i_size -= cnt;
    inode_table_update(inode);

    // success
    ret = 0;

out:
    // release memory
    if (NULL != buf) {
        sys_free(buf);
    }
    mutex_unlock(&ptn->mlock);
    return ret;
}


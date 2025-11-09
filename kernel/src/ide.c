#include "ide.h"
#include "thread.h"
#include "io.h"
#include "printk.h"
#include "string.h"
#include "interrupt.h"
#include "stdio.h"
#include "lock.h"
#include "timer.h"
#include "memory.h"
#include "assert.h"

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
struct ptn_table
{
    uint8_t     boot_flag;
    uint8_t     start_head;
    uint8_t     start_sector;
    uint8_t     start_cylinder;
    uint8_t     type;
    uint8_t     end_head;
    uint8_t     end_sector;
    uint8_t     end_cylinder;
    uint32_t    start_lba;
    uint32_t    sec_cnt;
} __attribute__ ((packed)); // avoid data alignment

// MBR or EBR
struct boot_sector
{
    uint8_t             boot_code[446];
    struct ptn_table    ptable[4];
    uint16_t            signature; //0x55AA
} __attribute__ ((packed)); // avoid data alignment

//=========================
// global variable
//=========================
struct ide_ch g_ide_ch[IDE_CH_NR];
struct list ptn_list;

//=========================
// internal functions
//=========================
static void ide_set_drive(struct ide_hd* hd)
{
    uint16_t port_base = hd->ch->port_base;
    uint16_t port;

    // set drive
    port = port_base + IDE_DRIVE_SET;
    if (0==hd->dev_nr) {
        outb(port, DRIVE_DEFAULT | DRIVE_MASTER | DRIVE_LBA);
    } else {
        outb(port, DRIVE_DEFAULT | DRIVE_SLAVE | DRIVE_LBA);
    }
}

static void ide_set_cmd(struct ide_hd* hd, uint8_t cmd)
{
    uint16_t port_base = hd->ch->port_base;
    uint16_t port;

    // set command
    port = port_base + IDE_STA_CMD;
    outb(port, cmd);
}

static void ide_set_lba(struct ide_hd* hd, uint32_t start, uint32_t cnt)
{
    uint16_t port_base = hd->ch->port_base;
    uint16_t port;
    uint8_t reg;

    // set start lba
    port = port_base + IDE_LBA_LOW;
    outb(port, start);

    port = port_base + IDE_LBA_MID;
    outb(port, start >> 8);

    port = port_base + IDE_LBA_HIGH;
    outb(port, start >> 16);

    port = port_base + IDE_DRIVE_SET;
    reg = inb(port);
    outb(port, reg | ((start >> 24) & 0xF));

    // set number of sectors, max is 256
    if (256 <= cnt) {
        cnt = 0; // 0 is a special value for 256
    }
    port = port_base + IDE_SEC_CNT;
    outb(port, cnt);
}

static void ide_set_data(struct ide_hd* hd, void* src, uint32_t cnt)
{
    uint16_t port_base = hd->ch->port_base;
    uint16_t port;
    uint32_t size = cnt * SECTOR_SIZE;

    port = port_base + IDE_DATA_REG;
    outsw(port, src, size / 2);
}

static void ide_get_data(struct ide_hd* hd, void* dst, uint32_t cnt)
{
    uint16_t port_base = hd->ch->port_base;
    uint16_t port;
    uint32_t size = cnt * SECTOR_SIZE;

    port = port_base + IDE_DATA_REG;
    insw(port, dst, size / 2);
}

static uint8_t ide_get_status(struct ide_hd* hd)
{
    uint16_t port_base = hd->ch->port_base;
    uint16_t port;

    port = port_base + IDE_STA_CMD;
    return inb(port);
}

static void ide_handler(uint8_t irq)
{
    uint32_t idx;

    for (idx=0; idx<IDE_CH_NR; idx++)
    {
        if (g_ide_ch[idx].irq == irq) {
            //pr_debug("sema_up, irq=0x%x\n", irq);
            sema_up(&g_ide_ch[idx].sema);
            break;
        }

    }
}

static bool ide_get_identify(struct ide_hd* hd)
{
    uint8_t id_info[512] = {0};
    uint8_t stat;

    // set drive
    ide_set_drive(hd);

    // set command
    ide_set_cmd(hd, CMD_IDENTIFY);

    // check status
    do
    {
        msleep(10);
        stat = ide_get_status(hd);
    }while (stat & STAT_BSY) ;

    // print debug log
    pr_debug("%s", hd->name);
    pr_debug("    stat: 0x%x", stat);
    pr_debug("    channel: %s\n", hd->ch->name);

    // check result
    if (0 == stat) {
        return false;
    }

    // clear semaphore
    sema_down(&hd->ch->sema);

    // read data
    ide_get_data(hd, id_info, 1);

    // print info
    uint32_t sec_num = *(uint32_t*)&id_info[ID_SEC_NUM];
    pr_debug("    Sectors: %d", sec_num);
    pr_debug("    Capacity: %d MB\n", sec_num * SECTOR_SIZE / 1024 / 1024);

    return true;
}

static void ide_get_partition(\
    struct ide_hd* hd, uint32_t base, uint32_t offset)
{
    struct boot_sector* bs = sys_malloc(sizeof(struct boot_sector));
    struct ptn_table* ptable = bs->ptable;
    uint32_t idx;
    static int ptn_nr = 0;

    // get MBR/EBR
    ide_read(hd, base + offset, bs, 1);

    for (idx=0; idx<4; idx++)
    {
        // unused
        if (0 == ptable->type) {
            ptable++;
            continue;
        }

        // extended partition using recursive
        if (0x5 == ptable->type) {
            // If the base is not 0, it must be in the extended partition.
            if (0 == base) {
                // for first EBR
                ptn_nr = 5;
                ide_get_partition(hd, ptable->start_lba, 0);
            } else {
                ide_get_partition(hd, base, ptable->start_lba);
            }

            // next partition table
            ptable++;
            continue;
        }

        // add to list
        if (ptable->type != 0) {
            pr_debug("    type:0x%x", ptable->type);

            // set partition table info
            struct ide_ptn* ptn = sys_malloc(sizeof(struct ide_ptn));
            if (0 == base) { // primary partition
                sprintf(ptn->name, "%s_%d", hd->name, idx+1);
            } else { // extended partition
                sprintf(ptn->name, "%s_%d", hd->name, ptn_nr++);
            }
            ptn->start_lba = base + offset + ptable->start_lba;
            ptn->sec_cnt = ptable->sec_cnt;
            ptn->hd = hd;
            list_append(&ptn_list, &ptn->ptn_tag);

            pr_debug(", name:%s, start:%d, sectors:%d\n",\
                ptn->name, ptn->start_lba, ptn->sec_cnt);
        }

        // next partition entry
        ptable++;
    }

    sys_free(bs);
}

static void ide_ch_init(void)
{
    uint32_t ch_idx;
    uint32_t hd_idx;

    pr_debug("%s +++\n", __func__);

    g_ide_ch[0].port_base   = 0x1F0;
    g_ide_ch[0].irq         = 0x20 + 14;

    g_ide_ch[1].port_base   = 0x170;
    g_ide_ch[1].irq         = 0x20 + 15;

    g_ide_ch[2].port_base   = 0x1E8;
    g_ide_ch[2].irq         = 0x20 + 11;

    g_ide_ch[3].port_base   = 0x168;
    g_ide_ch[3].irq         = 0x20 + 10;

    // data structure init
    for (ch_idx=0; ch_idx<IDE_CH_NR; ch_idx++)
    {
        sprintf(g_ide_ch[ch_idx].name, "ide%d", ch_idx);
        mutex_init(&g_ide_ch[ch_idx].mlock);
        sema_init(&g_ide_ch[ch_idx].sema, 0);
        register_handler(g_ide_ch[ch_idx].irq, ide_handler);

        // for each hard disk
        struct ide_hd* hd;
        for (hd_idx=0; hd_idx<2; hd_idx++)
        {
            hd = &g_ide_ch[ch_idx].dev[hd_idx];
            sprintf(hd->name, "sd%c", 'a' + ch_idx * 2 + hd_idx);
            hd->ch = &g_ide_ch[ch_idx];
            hd->dev_nr = hd_idx;

            // check hd
            if (ide_get_identify(hd)) {
                hd->valid = true;
                ide_get_partition(hd, 0, 0);
            } else {
                hd->valid = false;
            }

        } // for (hd_idx...
    }// for (ch_idx...

    pr_debug("%s ---\n", __func__);
}

//=========================
// external functions
//=========================
void ide_read(struct ide_hd* hd, uint32_t lba, void* dst, uint32_t cnt)
{
    uint8_t stat;
    uint32_t cnt_done = 0;
    uint32_t cnt_each;
    void*  p_dst = dst;

    //pr_debug("%s +++\n", __func__);

    // acquire channel lock
    mutex_lock(&hd->ch->mlock);

    // set drive
    ide_set_drive(hd);

    while (cnt_done < cnt)
    {
        // max cnt for each run is 256
        if ((cnt - cnt_done) >= 256) {
            cnt_each = 256;
        } else {
            cnt_each = cnt - cnt_done;
        }

        // set sectors
        ide_set_lba(hd, lba, cnt_each);

        // set command
        ide_set_cmd(hd, CMD_READ_SEC);

        // block wait for irq
        sema_down(&hd->ch->sema);

        // wait for complete data transfer
        do
        {
            msleep(10);
            stat = ide_get_status(hd);
        }while ((stat & STAT_BSY) || !(stat & STAT_DRQ));

        // read the data
        ide_get_data(hd, p_dst, cnt_each);

        // for next run
        cnt_done += cnt_each;
        p_dst = (void*)((uint32_t)p_dst + cnt_each * SECTOR_SIZE);
    } // while

    // release channel lock
    mutex_unlock(&hd->ch->mlock);

    //pr_debug("%s ---\n", __func__);
}

void ide_write(struct ide_hd* hd, uint32_t lba, void* src, uint32_t cnt)
{
    uint8_t stat;
    uint32_t cnt_done = 0;
    uint32_t cnt_each;
    void*  p_src = src;

    //pr_debug("%s +++\n", __func__);

    // acquire channel lock
    mutex_lock(&hd->ch->mlock);

    // set drive
    ide_set_drive(hd);

    while (cnt_done < cnt)
    {
        // max cnt for each run is 256
        if ((cnt - cnt_done) >= 256) {
            cnt_each = 256;
        } else {
            cnt_each = cnt - cnt_done;
        }

        // set sectors
        ide_set_lba(hd, lba, cnt_each);

        // set command
        ide_set_cmd(hd, CMD_WRITE_SEC);

        // wait for ready data transfer
        do
        {
            msleep(10);
            stat = ide_get_status(hd);
        }while ((stat & STAT_BSY) || !(stat & STAT_DRQ));

        // write the data
        ide_set_data(hd, p_src, cnt_each);

        // block wait for irq
        sema_down(&hd->ch->sema);

        // for next run
        cnt_done += cnt_each;
        p_src = (void*)((uint32_t)p_src + cnt_each * SECTOR_SIZE);
    } // while

    // release channel lock
    mutex_unlock(&hd->ch->mlock);

    //pr_debug("%s ---\n", __func__);
}

void ide_init(void)
{
    pr_debug("ide_init()\n");

    // partition list init
    list_init(&ptn_list);

    // channel init
    ide_ch_init();

}



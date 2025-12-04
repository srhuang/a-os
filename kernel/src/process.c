#include "process.h"
#include "stdint.h"
#include "printk.h"
#include "ide.h"
#include "memory.h"
#include "file.h"
#include "math.h"
#include "stddef.h"

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
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Word;

struct Elf32_Ehdr {
    unsigned char e_ident[16];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
};

struct Elf32_Phdr{
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
 } Elf32_Phdr;

enum segment_type {
    PT_NULL,
    PT_LOAD,
    PT_DYNAMIC,
    PT_INTERP,
    PT_NOTE,
    PT_SHLIB,
    PT_PHDR
};

//=========================
// global variable
//=========================


//=========================
// internal functions
//=========================
static void process_fs(uint32_t sec_start, const char* path)
{
    struct ide_hd* hd = &g_ide_ch[0].dev[0];

    // get file size from ELF
    void* buf = sys_malloc(SECTOR_SIZE);
    ide_read(hd, sec_start, buf, 1);
    struct Elf32_Ehdr* elf = (struct Elf32_Ehdr*)buf;
    uint32_t file_size = elf->e_shoff + (elf->e_shentsize * elf->e_shnum);
    sys_free(buf);
    /*/
    pr_debug("Start of section headers:%d\n", elf->e_shoff);
    pr_debug("Size of section headers:%d\n", elf->e_shentsize);
    pr_debug("Number of section headers:%d\n", elf->e_shnum);
    //*/

    // write file to file system
    buf = sys_malloc(DIV_ROUND_UP(file_size, SECTOR_SIZE) * SECTOR_SIZE);
    ide_read(hd, sec_start, buf, DIV_ROUND_UP(file_size, SECTOR_SIZE));
    int32_t fd = sys_open(path, O_RDWR | O_TRUNC);
    if (-1 == fd) {
        fd = sys_open(path, O_CREATE | O_RDWR | O_TRUNC);
    }
    sys_write(fd, buf, file_size);
    sys_free(buf);
    pr_debug("%s:Write File(%s) size:%d\n", __func__, path, file_size);
}

static void segment_load(int32_t fd, \
    uint32_t offset, uint32_t size, uint32_t vaddr)
{
    pr_debug("%s:offset=%d, size=%d, vaddr=0x%x\n", \
        __func__, offset, size, vaddr);
    // calculate the count of pages
    uint32_t vaddr_max = vaddr + size - 1;
    uint32_t pg_cnt = \
        (((vaddr_max & 0xFFFFF000) - (vaddr & 0xFFFFF000)) >> 12) + 1;
    //pr_debug("pg_cnt:%d\n", pg_cnt);

    // allocate user pages
    page_malloc(PF_USER, (void*)vaddr, pg_cnt);

    // load the text
    sys_lseek(fd, offset, SEEK_SET);
    sys_read(fd, (void*)vaddr, size);
}

static int32_t process_load(const char* path)
{
    int32_t ret = -1;
    int32_t fd = sys_open(path, O_RDONLY);
    if (fd == -1) {
        return -1;
    }

    // get elf
    void* buf = sys_malloc(sizeof(struct Elf32_Ehdr));
    sys_read(fd, buf, sizeof(struct Elf32_Ehdr));
    struct Elf32_Ehdr* elf = (struct Elf32_Ehdr*)buf;
    uint16_t phnum = elf->e_phnum;
    uint16_t phentsize = elf->e_phentsize;
    uint32_t phoff = elf->e_phoff;
    uint32_t entry = elf->e_entry;
    sys_free(buf);
    /*/
    pr_debug("Program header number:%d\n", phnum);
    pr_debug("Program header size:%d\n", phentsize);
    pr_debug("Program header offset:%d\n", phoff);
    //*/

    // parsing program header
    buf = sys_malloc(sizeof(struct Elf32_Phdr));
    uint32_t idx;
    for (idx = 0; idx < phnum; idx++)
    {
        // get program header
        sys_lseek(fd, phoff, SEEK_SET);
        sys_read(fd, buf, sizeof(struct Elf32_Phdr));
        struct Elf32_Phdr* ph = (struct Elf32_Phdr*)buf;
        /*/
        pr_debug("Program header type:%d\n", ph->p_type);
        pr_debug("Program header offset:%d\n", ph->p_offset);
        pr_debug("Program header vaddr:0x%x\n", ph->p_vaddr);
        pr_debug("Program header size:0x%x\n", ph->p_filesz);
        //*/

        // load segment
        if (PT_LOAD == ph->p_type) {
            segment_load(fd, ph->p_offset, ph->p_filesz, ph->p_vaddr);
        }

        // next program header
        phoff += phentsize;
    } // for
    sys_free(buf);

    // close fd
    sys_close(fd);

    // return address of entry point
    return entry;
}

//=========================
// external functions
//=========================
void process_init()
{
    printk("%s +++\n", __func__);

    // user init
    process_fs(USR_START_SECTOR, USR_INIT_PATH);
    int32_t entry = process_load(USR_INIT_PATH);
    if (-1 == entry) {
        printk("%s: process load fail.\n", __func__);
    }
    pr_debug("%s:entry:0x%x\n", __func__, entry);

    printk("%s ---\n", __func__);
}




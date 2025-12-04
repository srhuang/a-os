#include "print.h"
#include "string.h"
#include "bitmap.h"
#include "list.h"
#include "stddef.h"
#include "memory.h"
#include "interrupt.h"
#include "timer.h"
#include "thread.h"
#include "lock.h"
#include "printk.h"
#include "stdio.h"
#include "assert.h"
#include "ide.h"

//=========================
// print.h
//=========================
void test_put_char()
{
    int i;
    int row = 8;

    // test character
    put_char('C');

    // test CR
    put_char('\r');

    // test LF
    put_char('\n');

    // test backspace
    put_char('B');
    put_char('B');
    put_char('\b');

    // test roll screen
    put_char('\n');
    for(i=0; i<80*row; i++)
    {
        if( i%80 == 0 ){
            put_char('H');
        }else if ( i%80 == 79){
            put_char('E');
        }else{
            put_char('*');
        }
    }//for

}//test_put_char

void test_put_str()
{
    put_str("Test put string\n");
}

void test_put_int()
{
    put_int(0);
    put_char('\n');
    put_int(9);
    put_char('\n');
    put_int(0x12345678);
    put_char('\n');
    put_int(9527); // print in 16-bit: 0x2537
    put_char('\n');
    put_int(0x00005678);
    put_char('\n');
    put_int(0x00000000);
    put_char('\n');
}

void test_set_cursor()
{
    set_cursor(80*20);
}

void test_cls_screen()
{
    cls_screen();
}

//=========================
// string.h
//=========================
void test_string()
{
    put_str("test string\n");
    char src[10]={'\0'};
    char dst[10]={'\0'};

    // test memset()
    put_str("\n   test memset\n");
    int i;
    for(i=0; i<9; i++)
    {
        memset(src+i, '0'+i+1, 1);
    }
    memset(src+9, '\0', 1);
    put_str("src:");
    put_str(src);
    put_str("\n");
    put_str("dst:");
    put_str(dst);
    put_str("\n");

    // test memcpy()
    put_str("\n   test memcpy\n");
    memcpy(dst, src, 10);
    put_str("src:");
    put_str(src);
    put_str("\n");
    put_str("dst:");
    put_str(dst);
    put_str("\n");

    // test memcmp()
    put_str("\n   test memcmp\n");
    src[5]='0';
    if(0==memcmp(src, dst, 10))
    {
        put_str("The same\n");
    }else{
        put_str("Different\n");
    }
    put_str("src:");
    put_str(src);
    put_str("\n");
    put_str("dst:");
    put_str(dst);
    put_str("\n");

    // test strcpy()
    put_str("\n   test strcpy\n");
    strcpy(dst, src);
    put_str("src:");
    put_str(src);
    put_str("\n");
    put_str("dst:");
    put_str(dst);
    put_str("\n");

    // test strlen()
    put_str("\n   test strlen\n");
    uint32_t src_n = strlen(src);
    uint32_t dst_n = strlen(dst);
    put_str("src length:");
    put_int(src_n);
    put_str("\n");
    put_str("dst length:");
    put_int(dst_n);
    put_str("\n");

    //*
    // test strcmp()
    put_str("\n   test strcmp\n");
    src[5]='6';
    if(0==strcmp(src, dst))
    {
        put_str("The same\n");
    }else{
        put_str("Different\n");
    }
    put_str("src:");
    put_str(src);
    put_str("\n");
    put_str("dst:");
    put_str(dst);
    put_str("\n");

    // test strchr()
    put_str("\n   test strchr\n");
    char* str = "no pain no gain";
    char* first = strchr(str, 'a');
    put_str(str);
    put_str("\n");
    put_str("first 'a' is at:");
    put_int(first - str);
    put_str("\n");

    // test strrchr()
    put_str("\n   test strrchr\n");
    char* last = strrchr(str, 'a');
    put_str("last 'a' is at:");
    put_int(last - str);
    put_str("\n");

    // test strcat()
    put_str("\n   test strcat\n");
    char str1[50] = "no pain no gain, \0";
    char* str2 = "no sweat no sweet";
    strcat(str1, str2);
    put_str(str1);
    put_str("\n");

    // test strchrs()
    put_str("\n   test strchrs\n");
    int n = strchrs(str1, 'n');
    put_str("count of 'n':");
    put_int(n);
    put_str("\n");
    //*/
}

//=========================
// bitmap.h
//=========================
void test_bitmap()
{
    char test[4] ={0x1, 0x2, 0x3, 0x4};
    struct bitmap btmp;

    // test bitmap_init()
    put_str("test bitmap_init\n");
    bitmap_init(&btmp, test, 4);
    put_str("bitmap: ");
    put_int(test[0]);
    put_int(test[1]);
    put_int(test[2]);
    put_int(test[3]);
    put_char('\n');

    // test bitmap_reset()
    put_str("test bitmap_reset\n");
    bitmap_reset(&btmp);
    put_str("bitmap: ");
    put_int(test[0]);
    put_int(test[1]);
    put_int(test[2]);
    put_int(test[3]);
    put_char('\n');

    // test bitmap_set()
    put_str("test bitmap_set\n");
    bitmap_set(&btmp, 12, true);
    put_str("bitmap[0]: ");
    put_int(test[0]);
    put_str("\nbitmap[1]: ");
    put_int(test[1]);
    put_str("\nbitmap[2]: ");
    put_int(test[2]);
    put_str("\nbitmap[3]: ");
    put_int(test[3]);
    put_char('\n');

    // test bitmap_check()
    put_str("test bitmap_check\n");
    int value = bitmap_check(&btmp, 11);
    put_str("bitmap[11]= ");
    put_int(value);
    put_char('\n');

    value = bitmap_check(&btmp, 12);
    put_str("bitmap[12]= ");
    put_int(value);
    put_char('\n');

    // test bitmap_aquire()
    put_str("test bitmap_acquire\n");
    put_str("acquire 1, ");
    int idx = bitmap_acquire(&btmp, 1);
    put_str("free_idx: ");
    put_int(idx);
    put_char('\n');

    put_str("acquire 11, ");
    idx = bitmap_acquire(&btmp, 11);
    put_str("free_idx: ");
    put_int(idx);
    put_char('\n');

    put_str("acquire 19, ");
    idx = bitmap_acquire(&btmp, 19);
    put_str("free_idx: ");
    put_int(idx);
    put_char('\n');

    put_str("acquire 12, ");
    idx = bitmap_acquire(&btmp, 12);
    put_str("free_idx: ");
    put_int(idx);
    put_char('\n');

    //*
    put_str("test bitmap_release\n");
    bitmap_release(&btmp, 16, 4);
    //*/

    put_str("bitmap[0]: ");
    put_int(test[0]);
    put_str("\nbitmap[1]: ");
    put_int(test[1]);
    put_str("\nbitmap[2]: ");
    put_int(test[2]);
    put_str("\nbitmap[3]: ");
    put_int(test[3]);
    put_char('\n');
}

//=========================
// list.h
//=========================
struct test_item{
    int id;
    struct list_elem tag;
};

static bool check_id(struct list_elem* elem, int arg)
{
    struct test_item* bbb = elem2entry(struct test_item, tag, elem);
    if (bbb->id == arg) {
        return true;
    }
    return false;
}

void test_list()
{
    struct list all_tag;
    struct test_item node1, node2, node3, node4, node5;
    struct list_elem* get_tag;
    struct test_item* get_item;

    node1.id = 0x111;
    node2.id = 0x222;
    node3.id = 0x333;
    node4.id = 0x444;
    node5.id = 0x555;

    put_str("test list_init\n");
    list_init(&all_tag);

    //*
    put_str("test list_insert\n");
    list_insert(&all_tag.tail, &node1.tag);
    //*/

    /*
    put_str("test list_remove\n");
    list_remove(&node1.tag);
    //*/

    /*
    put_str("test list_append\n");
    list_append(&all_tag, &node2.tag);
    //*/

    /*
    put_str("test list_push\n");
    list_push(&all_tag, &node3.tag);
    list_push(&all_tag, &node4.tag);
    //*/

    /*
    put_str("test list_pop\n");
    get_tag = list_pop(&all_tag);
    put_str("pop tag: ");
    if (NULL == get_tag) {
        put_str("NULL");
    } else {
        get_item = elem2entry(struct test_item, tag, get_tag);
        put_int(get_item->id);
    }
    put_str("\n");
    //*/

    /*
    put_str("test list_empty\n");
    if(list_empty(&all_tag)) {
        put_str("list is empty.\n");
    } else {
        put_str("list is NOT empty.\n");
    }
    //*/

    /*
    put_str("test list_len\n");
    int len = list_len(&all_tag);
    put_str("list length: ");
    put_int(len);
    put_str("\n");
    //*/

    /*
    put_str("test list_find\n");
    if (list_find(&all_tag, &node3.tag)) {
        put_str("Found.\n");
    } else {
        put_str("NOT Found.\n");
    }
    //*/

    /*
    put_str("test list_traversal\n");
    get_tag = list_traversal(&all_tag, check_id, 0x333);
    if (NULL == get_tag) {
        put_str("NOT Found\n");
    } else {
        get_item = elem2entry(struct test_item, tag, get_tag);
        put_str("Found: ");
        put_int(get_item->id);
        put_str("\n");
    }
    //*/

    // print all list
    struct list_elem* cur_elem = all_tag.head.next;
    struct test_item* aaa;
    put_str("list: head->");
    while (cur_elem != &all_tag.tail)
    {
        aaa = elem2entry(struct test_item, tag, cur_elem);
        put_int(aaa->id);
        put_str("->");
        cur_elem = cur_elem->next;
    }
    put_str("tail\n");
}

//=========================
// memory.h
//=========================
void test_memory()
{
    put_str("test page_malloc(PF_KERNEL, 3): ");
    void* vaddr_1 = page_malloc(PF_KERNEL, NULL, 3);
    put_int((uint32_t)vaddr_1);
    put_str("\n");

    put_str("test page_malloc(0xC0102000, 3): ");
    void* vaddr_2 = page_malloc(PF_KERNEL, (void*)0xC0102000, 3);
    put_int((uint32_t)vaddr_2);
    put_str("\n");

    put_str("test page_malloc(0xC0107000, 3): ");
    void* vaddr_3 = page_malloc(PF_KERNEL, (void*)0xC0107000, 3);
    put_int((uint32_t)vaddr_3);
    put_str("\n");

    /*
    put_str("test page_free\n");
    page_free(PF_KERNEL, vaddr_1, 2);
    //*/

    put_str("test sys_malloc(31): ");
    void* vaddr_4 = sys_malloc(31);
    put_int((uint32_t)vaddr_4);
    put_str("\n");

    //*
    put_str("test sys_malloc(29): ");
    void* vaddr_5 = sys_malloc(29);
    put_int((uint32_t)vaddr_5);
    put_str("\n");
    //*/

    /*
    extern struct mem_block_desc k_mem_block[MEM_BLOCK_CNT];
    put_str("test mem_block free list\n");
    struct list* plist;
    struct list_elem* cur_elem;
    int idx;
    for (idx=0; idx<MEM_BLOCK_CNT; idx++)
    {
        put_int(k_mem_block[idx].block_size);
        put_str(" byte free list: ");

        plist = &k_mem_block[idx].free_list;
        cur_elem = plist->head.next;
        int cnt = 0;
        while (cur_elem != &plist->tail)
        {
            cnt++;
            cur_elem = cur_elem->next;
        }
        put_int(cnt);
        put_str("\n");
    }
    //*/

    //*
    put_str("test sys_free\n");
    sys_free(vaddr_4);
    //*/
}

void test_intr()
{
    /* test page fault
    uint32_t* addr = (uint32_t*)0xC0200000;
    uint32_t val = *addr;
    //*/

    /* test divide zero
    int dzero = 9527/0;
    //*/

    /*
    asm volatile("int $0x1F");
    //*/

    //* enable interrupt
    intr_set_status(true);
    //*/

    /* disable interrupt
    intr_set_status(false);
    //*/

    // get interrupt status
    put_str("intr_get_statu: ");
    bool ret = intr_get_status();
    put_int(ret);
    put_str("\n");

    /* test system call
    extern void test_syscall0(void);
    extern int test_syscall1(int a);
    extern int test_syscall2(int a, int b);
    extern int test_syscall3(int a, int b, int c);

    put_str("test syscall0()\n");
    test_syscall0();
    int sysret;

    put_str("test_syscall1()\n");
    sysret = test_syscall1(0x111);
    put_str("ret= 0x");
    put_int(sysret);
    put_str("\n");

    put_str("test_syscall2()\n");
    sysret = test_syscall2(0x111, 0x222);
    put_str("ret= 0x");
    put_int(sysret);
    put_str("\n");

    put_str("test_syscall3()\n");
    sysret = test_syscall3(0x111, 0x222, 0x333);
    put_str("ret= 0x");
    put_int(sysret);
    put_str("\n");
    //*/

}

void test_timer()
{
    put_str("100 ms to jiffies: 0x");
    put_int(msecs_to_jiffies(100));
    put_str("\n");

    put_str("100 us to jiffies: 0x");
    put_int(usecs_to_jiffies(100));
    put_str("\n");

    put_str("5000 jiffies to ms: 0x");
    put_int(jiffies_to_msecs(5000));
    put_str("\n");

    put_str("50 jiffies to us: 0x");
    put_int(jiffies_to_usecs(50));
    put_str("\n");

    put_str("jiffies: 0x");
    put_int(jiffies);
    put_str("\n");


}

struct mutex mlock;
struct semaphore sema;
#define LOCK_SEMA   (0)
void thread_aaa(void* arg)
{
    uint32_t idx = 0;
    while(1)
    {
        msleep(1000);

#if (LOCK_SEMA)
        sema_down(&sema);
#else
        mutex_lock(&mlock);
#endif
        set_cursor(160);
        put_str(arg);
        put_str(":0x");
        put_int(idx++);
        put_str(" ");

#if (LOCK_SEMA)
        sema_up(&sema);
#else
        mutex_unlock(&mlock);
#endif
    }
}

void thread_bbb(void* arg)
{
    uint32_t idx = 0;
    while(1)
    {
        msleep(100);

#if (LOCK_SEMA)
        sema_down(&sema);
#else
        mutex_lock(&mlock);
#endif
        set_cursor(240);
        put_str(arg);
        put_str(":0x");
        put_int(idx++);
        put_str(" ");

#if (LOCK_SEMA)
        sema_up(&sema);
#else
        mutex_unlock(&mlock);
#endif
    }
}

void thread_ccc(void* arg)
{
    uint32_t idx = 0;
    while(1)
    {
        msleep(10);

#if (LOCK_SEMA)
        sema_down(&sema);
#else
        mutex_lock(&mlock);
#endif
        set_cursor(320);
        put_str(arg);
        put_str(":0x");
        put_int(idx++);
        put_str(" ");

#if (LOCK_SEMA)
        sema_up(&sema);
#else
        mutex_unlock(&mlock);
#endif
    }
}

void thread_ddd(void* arg)
{
    uint32_t idx = 0;
    while(1)
    {
        usleep(1000);

#if (LOCK_SEMA)
        sema_down(&sema);
#else
        mutex_lock(&mlock);
#endif
        set_cursor(400);
        put_str(arg);
        put_str(":0x");
        put_int(idx++);
        put_str(" ");

#if (LOCK_SEMA)
        sema_up(&sema);
#else
        mutex_unlock(&mlock);
#endif
    }
}

void thread_eee(void* arg)
{
    uint32_t idx = 0;
    while(1)
    {
        usleep(100);

#if (LOCK_SEMA)
        sema_down(&sema);
#else
        mutex_lock(&mlock);
#endif
        set_cursor(480);
        put_str(arg);
        put_str(":0x");
        put_int(idx++);
        put_str(" ");

#if (LOCK_SEMA)
        sema_up(&sema);
#else
        mutex_unlock(&mlock);
#endif
    }
}

void thread_fff(void* arg)
{
    uint32_t idx = 0;
    while(1)
    {
        usleep(10);

#if (LOCK_SEMA)
        sema_down(&sema);
#else
        mutex_lock(&mlock);
#endif
        set_cursor(560);
        put_str(arg);
        put_str(":0x");
        put_int(idx++);
        put_str(" ");

#if (LOCK_SEMA)
        sema_up(&sema);
#else
        mutex_unlock(&mlock);
#endif
    }
}

void thread_idle(void* arg)
{
    while(1)
    {
        msleep(1000);
        kthread_unblock(g_idle_task);
    }
}

void test_thread()
{
    struct task_struct* taskA = kthread_create(thread_aaa, "A", "kthreadA");
    struct task_struct* taskB = kthread_create(thread_bbb, "B", "kthreadB");
    struct task_struct* taskC = kthread_create(thread_ccc, "C", "kthreadC");
    struct task_struct* taskD = kthread_create(thread_ddd, "D", "kthreadD");
    struct task_struct* taskE = kthread_create(thread_eee, "E", "kthreadE");
    struct task_struct* taskF = kthread_create(thread_fff, "F", "kthreadF");

    // test block/unblock
    struct task_struct* task_idle = \
        kthread_create(thread_idle, "IDLE", "kthreadI");

    cls_screen();

    // init lock
#if (LOCK_SEMA)
    put_str("test semaphore\n");
    sema_init(&sema, 1);
#else
    put_str("test mutex\n");
    mutex_init(&mlock);
#endif

    kthread_run(taskA);
    kthread_run(taskB);
    kthread_run(taskC);
    kthread_run(taskD);
    kthread_run(taskE);
    kthread_run(taskF);
    //kthread_run(task_idle);
}

void test_printk()
{
    int aaa = 9527;
    char* name = "srhuang";
    char c = 'S';
    printk("test_printk:%d, 0x%x, %s, [%c]\n", aaa, aaa, name, c);

    char buf[1024] = {0};
    sprintf(buf, "test_sprintf:%d, 0x%x, %s, [%c]\n",\
        aaa, aaa, name, c);
    put_str(buf);
}

void thread_xxx(void* arg)
{
    uint32_t count = 0;

    while (1)
    {
        /*
        printk("Thread %s:%d ", arg, count++);
        //*/

        /*
        void* vaddr = page_malloc(PF_KERNEL, NULL, 10);
        page_free(PF_KERNEL, vaddr, 10);
        //*/

        //*
        void* vaddr = sys_malloc(31);
        sys_free(vaddr);
        //*/
    }
}

void thread_yyy(void* arg)
{
    uint32_t count = 0;

    while (1)
    {
        /*
        printk("Thread %s:%d ", arg, count++);
        //*/

        /*
        void* vaddr = page_malloc(PF_KERNEL, NULL, 10);
        page_free(PF_KERNEL, vaddr, 10);
        //*/

        //*
        void* vaddr = sys_malloc(31);
        sys_free(vaddr);
        //*/
    }
}

void thread_zzz(void* arg)
{
    uint32_t count = 0;

    while (1)
    {
        /*
        printk("Thread %s:%d ", arg, count++);
        //*/

        /*
        void* vaddr = page_malloc(PF_KERNEL, NULL, 10);
        page_free(PF_KERNEL, vaddr, 10);
        //*/

        //*
        void* vaddr = sys_malloc(31);
        sys_free(vaddr);
        //*/
    }
}

void test_concurrency()
{
    struct task_struct* taskX = kthread_create(thread_xxx, "XXX", "kthreadX");
    struct task_struct* taskY = kthread_create(thread_yyy, "YYY", "kthreadY");
    struct task_struct* taskZ = kthread_create(thread_zzz, "ZZZ", "kthreadZ");

    kthread_run(taskX);
    kthread_run(taskY);
    kthread_run(taskZ);
}

void test_ide()
{
    uint8_t aaa[512] = {0x95, 0x27};
    uint8_t bbb[512] = {0};
    uint8_t buf[512];
    struct ide_ptn* ptn = \
        elem2entry(struct ide_ptn, ptn_tag, ptn_list.head.next);
    struct ide_hd* hd = ptn->hd;

    ide_read(hd, 1, buf, 1);
    printk("before write:0x%x, 0x%x\n", buf[0], buf[1]);

    ide_write(hd, 1, aaa, 1);

    ide_read(hd, 1, buf, 1);
    printk("after write:0x%x, 0x%x\n", buf[0], buf[1]);

    // recover
    ide_write(hd, 1, bbb, 1);

}

#include "fs.h"
void test_fs()
{
    struct ide_ptn* ptn;
    struct list_elem* cur_elem;

    printk("%s +++\n", __func__);

    // print root_dir
    printk("root_dir:");
    struct dirent* dir = (struct dirent*)root_dir.inode->i_block[0];
    int dir_idx=0;
    while (dir_idx < ROOT_DIR_MAX)
    {
        if (FT_UNKNOWN != (dir + dir_idx)->f_type) {
            printk("%s ", (dir + dir_idx)->filename);
        }
        dir_idx++;
    } // while
    printk("\n");

    // test fs_unmount(), fs_format(), fs_unmount()
    cur_elem = ptn_list.head.next;
    while (cur_elem != &ptn_list.tail)
    {
        ptn = elem2entry(struct ide_ptn, ptn_tag, cur_elem);

        fs_unmount(ptn);
        fs_format(ptn);
        fs_mount(ptn);

        // next partition
        cur_elem = cur_elem->next;
    }

    // print root_dir
    printk("root_dir:");
    dir_idx=0;
    while (dir_idx < ROOT_DIR_MAX)
    {
        if (FT_UNKNOWN != (dir + dir_idx)->f_type) {
            printk("%s ", (dir + dir_idx)->filename);
        }
        dir_idx++;
    } // while
    printk("\n");

    printk("%s ---\n", __func__);
}

#include "inode.h"
void test_inode()
{
    struct inode_sys* inode;
    struct ide_ptn* ptn = \
        elem2entry(struct ide_ptn, ptn_tag, ptn_list.head.next->next);

    printk("%s +++\n", __func__);

    //* clear file system on the partition
    fs_unmount(ptn);
    fs_format(ptn);
    fs_mount(ptn);
    //*/

    // test inode_open(), inode_close()
    printk("test inode_open()/inode_close()\n");
    inode = inode_open(4097);
    printk("i_no=%d, i_size=%d, i_block[0]=%d\n", \
        inode->i_no, inode->i_size, inode->i_block[0]);
    //inode_close(inode);

    // print all open nodes
    printk("print all open inodes: ");
    struct list_elem* elem = ptn->open_inodes.head.next;
    struct inode_sys* elem_inode;
    while (elem != &ptn->open_inodes.tail)
    {
        elem_inode = elem2entry(struct inode_sys, inode_tag, elem);
        printk("%d, ", elem_inode->i_no);

        elem = elem->next;
    }
    printk("\n");

    // test inode_acquire(), inode_release()
    printk("test inode_acquire()/inode_release()\n");
    inode = inode_acquire(ptn);
    printk("new inode table:%d\n", inode->i_no);

    // print all open nodes
    printk("print all open inodes: ");
    elem = ptn->open_inodes.head.next;
    elem_inode;
    while (elem != &ptn->open_inodes.tail)
    {
        elem_inode = elem2entry(struct inode_sys, inode_tag, elem);
        printk("%d, ", elem_inode->i_no);

        elem = elem->next;
    }
    printk("\n");
    //inode_release(inode);

    printk("inode_read()/inode_write()\n");
    // prepare the data
    void* buf = sys_malloc(BLOCK_SIZE*2);
    uint8_t* p_buf = (uint8_t*)buf;
    int idx = 0;
    while (idx < BLOCK_SIZE*2)
    {
        *p_buf = idx % 256;
        p_buf++;
        idx++;
    }

    // write data to hard disk
    inode_write(inode, 2048, (uint8_t*)buf, BLOCK_SIZE*2);

    printk("inode re-open\n");
    inode_close(inode);
    inode = inode_open(4098);

    // read data from hard disk
    inode_read(inode, 4000, (uint8_t*)buf, 10);
    idx = 0;
    p_buf = (uint8_t*)buf;
    printk("inode_read:");
    while (idx < 10)
    {
        printk("0x%x ", *p_buf);
        p_buf++;
        idx++;
    }
    printk("\n");

    inode_read(inode, 2048 + (BLOCK_SIZE*2) - 10, (uint8_t*)buf, 10);
    idx = 0;
    p_buf = (uint8_t*)buf;
    printk("inode_read:");
    while (idx < 10)
    {
        printk("0x%x ", *p_buf);
        p_buf++;
        idx++;
    }
    printk("\n");

    printk("inode_erase()\n");
    inode_erase(inode, BLOCK_SIZE);

    inode_read(inode, 2048 + (BLOCK_SIZE*2) - 10, (uint8_t*)buf, 10);

    inode_close(inode);
    inode = inode_open(4098);
    printk("test updating inode table, size=%d\n", inode->i_size);

    sys_free(buf);

    printk("%s ---\n", __func__);
}

#include "dir.h"
void test_dir()
{
    struct inode_sys* inode;
    struct ide_ptn* ptn = \
        elem2entry(struct ide_ptn, ptn_tag, ptn_list.head.next->next);
    struct dirstream* dir;
    struct dirent* dir_entry;

    printk("%s +++\n", __func__);

    //* clear file system on the partition
    fs_unmount(ptn);
    fs_format(ptn);
    fs_mount(ptn);
    //*/

    // test sys_opendir(), sys_closedir(), sys_readdir(), sys_rewinddir()
    dir = sys_opendir("/");
    printk("root:");
    while ((dir_entry = sys_readdir(dir)) != NULL)
    {
        printk("%s ", dir_entry->filename);
    }
    printk("\n");

    sys_rewinddir(dir);

    printk("root:");
    while ((dir_entry = sys_readdir(dir)) != NULL)
    {
        printk("%s ", dir_entry->filename);
    }
    printk("\n");
    sys_closedir(dir);

    dir = sys_opendir("/sdb_5");
    printk("/sdb_5: ");
    while ((dir_entry = sys_readdir(dir)) != NULL)
    {
        printk("%s(%d) ", dir_entry->filename, dir_entry->i_no);
    }
    printk("\n");
    sys_closedir(dir);


    // test sys_mkdir(), sys_rmdir()
    sys_mkdir("/sdb_5/aaa/");
    sys_mkdir("/sdb_5/bbb/");
    sys_mkdir("/sdb_5/ccc/");
    sys_rmdir("/sdb_5/aaa");

    dir = sys_opendir("/sdb_5");
    printk("/sdb_5: ");
    while ((dir_entry = sys_readdir(dir)) != NULL)
    {
        printk("%s(%d) ", dir_entry->filename, dir_entry->i_no);
    }
    printk("\n");
    sys_closedir(dir);

    sys_mkdir("/sdb_5/bbb/111");

    dir = sys_opendir("/sdb_5/bbb/");
    printk("/sdb_5/bbb/: ");
    while ((dir_entry = sys_readdir(dir)) != NULL)
    {
        printk("%s(%d) ", dir_entry->filename, dir_entry->i_no);
    }
    printk("\n");
    sys_closedir(dir);

    printk("%s ---\n", __func__);
}

#include "file.h"
void test_file()
{
    printk("%s +++\n", __func__);

    //* clear file system on the partition
    printk("\nformat the partition\n");
    struct ide_ptn* ptn = \
        elem2entry(struct ide_ptn, ptn_tag, ptn_list.head.next->next);
    fs_unmount(ptn);
    fs_format(ptn);
    fs_mount(ptn);
    //*/

    struct dirstream* dir;
    struct dirent* dir_entry;

    // test sys_open(), sys_close(), sys_unlink()
    printk("\ntest sys_open(), sys_close(), sys_unlink()\n");
    int32_t fd = sys_open("/sdb_5/bbb", O_CREATE | O_APPEND);
    struct task_struct* task = kthread_current();
    printk("fd=%d, fd_idx=%d\n", fd, task->open_fd[fd]);
    //sys_open("/sdb_5/bbb", 0);
    //sys_close(fd);
    //sys_unlink("/sdb_5/bbb");

    dir = sys_opendir("/sdb_5");
    printk("/sdb_5: ");
    while ((dir_entry = sys_readdir(dir)) != NULL)
    {
        printk("%s(%d) ", dir_entry->filename, dir_entry->i_no);
    }
    printk("\n");
    sys_closedir(dir);

    // test sys_write(), sys_read()
    printk("\ntest sys_write(), sys_read()\n");
    void* buf = sys_malloc(BLOCK_SIZE*2);
    uint8_t* p_buf = (uint8_t*)buf;
    int idx = 0;
    while (idx < BLOCK_SIZE*2)
    {
        *p_buf = idx % 256;
        p_buf++;
        idx++;
    }
    //sys_write(fd, buf, 5);
    sys_write(fd, buf, BLOCK_SIZE*2);
    memset(buf, 0, BLOCK_SIZE*2);

    sys_read(fd, buf, 10);
    idx = 0;
    p_buf = (uint8_t*)buf;
    printk("sys_read:");
    while (idx < 10)
    {
        printk("0x%x ", *p_buf);
        p_buf++;
        idx++;
    }
    printk("\n");

    // test sys_lseek()
    printk("\ntest sys_lseek()\n");
    sys_lseek(fd, 15, SEEK_SET);
    //sys_lseek(fd, 15, SEEK_CUR);
    //sys_lseek(fd, -10, SEEK_END);
    sys_read(fd, buf, 10);
    idx = 0;
    p_buf = (uint8_t*)buf;
    printk("sys_read:");
    while (idx < 10)
    {
        printk("0x%x ", *p_buf);
        p_buf++;
        idx++;
    }
    printk("\n");

    printk("%s ---\n", __func__);
}

//=========================
// test_all
//=========================
void test_all()
{
    /* print.h
    test_put_char();
    test_put_str();
    test_put_int();
    test_set_cursor();
    test_cls_screen();
    //*/

    /* string.h
    test_string();
    //*/

    /* bitmap.h
    test_bitmap();
    //*/

    /* list.h
    test_list();
    //*/

    /* memory.h
    test_memory();
    //*/

    /* interrupt.h
    test_intr();
    //*/

    /* timer.h
    test_timer();
    //*/

    /* thread.h and lock.h
    test_thread();
    //*/

    /* printk.h and stdio.h
    test_printk();
    //*/

    /*
    test_concurrency();
    //*/

    /* test assert
    assert(1==2);
    //*/

    /* ide.h
    test_ide();
    //*/

    /* fs.h
    test_fs();
    //*/

    /* inode.h
    test_inode();
    //*/

    /* dir.h
    test_dir();
    //*/

    /* file.h
    test_file();
    //*/
}


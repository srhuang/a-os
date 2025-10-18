#include "print.h"
#include "string.h"
#include "bitmap.h"
#include "list.h"
#include "stddef.h"
#include "memory.h"
#include "interrupt.h"

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

    //* interrupt.h
    test_intr();
    //*/
}


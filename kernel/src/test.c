#include "print.h"
#include "string.h"
#include "bitmap.h"

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

    //* bitmap.h
    test_bitmap();
    //*/
}


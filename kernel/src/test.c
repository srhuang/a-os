#include "print.h"

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

void test_all()
{
    // print.h
    test_put_char();
    test_put_str();
    test_put_int();
    //test_set_cursor();
    //test_cls_screen();
}


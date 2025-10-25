#include "test.h"
#include "print.h"
#include "init.h"
#include "thread.h"
#include "sched.h"
int main(void) {

    put_str("\nKernel main()\n");
    kernel_init();

    // test functions
    test_all();

    //*
    kthread_exit();
    //*/

    //while(1);

    return 0;
}

#include "test.h"
#include "print.h"
#include "init.h"

int main(void) {

    put_str("\nKernel main()\n");
    kernel_init();

    // test functions
    test_all();

    while(1);
    return 0;
}

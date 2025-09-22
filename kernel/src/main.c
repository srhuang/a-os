#include "test.h"
#include "print.h"
int main(void) {

    put_str("\nKernel main()\n");

    // test functions
    test_all();

    while(1);
    return 0;
}

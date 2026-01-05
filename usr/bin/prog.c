#include "printf.h"

int main(int argc, char** argv)
{
    printf("I am program\n");
    printf("argc=%d\n", argc);
    uint32_t idx;
    for (idx = 0; idx < argc; idx++)
    {
        printf("argv[%d]=%s\n", idx, argv[idx]);
    }
    return 0;
}


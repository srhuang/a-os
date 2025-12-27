#include "printf.h"
#include "syscall_usr.h"
#include "stddef.h"

void init(void)
{
    printf("User Process Init\n");
    ps();

    int16_t pid = fork();

    if (pid) {
        printf("I am parent\n");
    } else {
        printf("I am child\n");
        char* argv[16] = {NULL};
        exec("/sdb_1/bin/prog", argv);
    }

    while(1);

}

#include "printf.h"
#include "syscall_usr.h"

void init(void)
{
    printf("User Process Init\n");
    ps();

    int16_t pid = fork();

    if (pid) {
        printf("I am parent\n");
    } else {
        printf("I am child\n");
    }

    while(1);

}

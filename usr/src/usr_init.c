#include "printf.h"
#include "syscall_usr.h"
#include "stddef.h"

void init(void)
{
    printf("User Process Init\n");
    //ps();

    int16_t pid = fork();

    if (pid) {
        printf("I am User Process Init\n");

        // Keep reaping zombie processes here.
        int32_t status;
        int16_t pid;
        while(1)
        {
            pid = wait(&status);
            if (-1 != pid) {
                printf("reaping pid=%d\n", pid);
            }
        }
    } else {
        printf("I am child\n");
        char* argv[16] = {NULL};
        exec("/sdb_1/bin/prog", argv);
    }

    while(1);
}

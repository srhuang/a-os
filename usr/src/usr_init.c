#include "printf.h"
#include "syscall_usr.h"
#include "stddef.h"
#include "string.h"

void init(void)
{
    printf("User Process Init\n");
    //ps();

    int32_t pipefd[2] = {-1};
    pipe(pipefd);

    int16_t pid = fork();

    if (pid) {
        printf("I am User Process Init\n");

        // test pipe
        close(pipefd[0]);
        uint8_t* message = "hello son, i'm your father~";
        write(pipefd[1], message, strlen(message));

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

        // test pipe
        close(pipefd[1]);
        uint8_t buf[128];
        read(pipefd[0], buf, sizeof(buf));
        printf("my father said to me:%s\n", buf);

        while (read(0, buf, 1))
        {
            if (*buf == '\r') {
                printf("\n");
                break;
            } else {
                printf("%c", *buf);
            }
        }

        char* argv[16] = {NULL};
        exec("/sdb_1/bin/prog", argv);
    }

    while(1);
}

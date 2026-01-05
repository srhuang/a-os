#include "printf.h"
#include "syscall_usr.h"
#include "stddef.h"
#include "string.h"
#include "shell.h"

void init(void)
{
    printf("User Process Init\n");
    //ps();

    /* test pipe
    int32_t pipefd[2] = {-1};
    pipe(pipefd);
    //*/

    int16_t pid = fork();

    if (pid) {
        //printf("I am User Process Init\n");

        /* test pipe
        close(pipefd[0]);
        uint8_t* message = "hello son, i'm your father~";
        write(pipefd[1], message, strlen(message));
        //*/

        // Keep reaping zombie processes here.
        int32_t status;
        int16_t pid;
        while(1)
        {
            pid = wait(&status);
            if (-1 != pid) {
                //printf("reaping pid=%d\n", pid);
            }
        }
    } else {
        //printf("I am child\n");

        /* test pipe
        close(pipefd[1]);
        uint8_t buf[128];
        read(pipefd[0], buf, sizeof(buf));
        printf("my father said to me:%s\n", buf);
        //*/

        /* test keyboard
        while (read(0, buf, 1))
        {
            if (*buf == '\r') {
                printf("\n");
                break;
            } else {
                printf("%c", *buf);
            }
        }
        //*/

        /* test exec
        uint32_t argv_size = ARG_NR_MAX * sizeof(char*);
        char** argv= malloc(argv_size);
        memset(argv, 0, argv_size);

        argv[0]= malloc(PATH_LEN_MAX);
        strcpy(argv[0], "/sdb_1/bin/prog");
        argv[1]= malloc(CMD_LEN_MAX);
        strcpy(argv[1], "aaa");
        exec(argv[0], argv);
        //*/

        myshell();
    }

    while(1);
}

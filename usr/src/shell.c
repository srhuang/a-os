#include "shell.h"
#include "printf.h"
#include "syscall_usr.h"
#include "stdio.h"
#include "string.h"
#include "buildin_cmd.h"
#include "path.h"

//=========================
// debugging
//=========================
//#define DEBUG

#ifdef DEBUG
    #define pr_debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define pr_debug(fmt, ...) do { } while (0)
#endif

//=========================
// internal struct
//=========================

//=========================
// global variable
//=========================

//=========================
// internal functions
//=========================
static void readline(char* buf, uint32_t size)
{
    char* pos = buf;
    while (read(stdin_no, pos, 1))
    {
        // check size
        if ((pos - buf + 1) > size) {
            break;
        }

        switch(*pos)
        {
            case '\n':
            case '\r':
                *pos = 0;
                printf("\n");
                return;
            case '\b':
                if (pos > buf) {
                    printf("\b");
                    pos--;
                }
                break;
            case KB_CTRL_L:
                *pos = 0;
                clear();
                uint8_t cwd[PATH_LEN_MAX];
                getcwd(cwd, PATH_LEN_MAX);
                printf("srhuang@localhost:%s$", cwd);
                printf("%s", buf);
                break;
            case KB_CTRL_U:
                while (pos > buf)
                {
                    printf("\b");
                    *pos-- = 0;
                }
                break;
            default:
                printf("%c", *pos);
                pos++;
        } // switch
    } // while
}

static int32_t cmd_parse(char* cmd, char** argv)
{
    int32_t argc = 0;
    char* p = cmd;
    char token = ' ';
    char* str = NULL;

    pr_debug("%s:cmd=%s\n", __func__, cmd);

    while (*p)
    {
        // skip tokens
        while (*p == token)
        {
            p++;
        }
        if (0 == *p) {
            break;
        }

        // get the argv
        str = p;
        while (*p)
        {
            if (*p == token)
            {
                *p++ = 0;
                break;
            }
            p++;
        }
        argv[argc] = malloc(PATH_LEN_MAX);
        strcpy(argv[argc], str);
        pr_debug("%s:argv[%d]=%s\n", __func__, argc, argv[argc]);
        argc++;

        // check argc
        if (ARG_NR_MAX == argc) {
            break;
        }
    }

    return argc;
}

static void cmd_exec(uint32_t argc, char** argv)
{
    pr_debug("%s:argv[0]=%s\n", __func__, argv[0]);

    // build-in command
    if (!strcmp("clear", argv[0])) {
        buildin_clear(argc, argv);
        return;
    }
    if (!strcmp("ps", argv[0])) {
        buildin_ps(argc, argv);
        return;
    }
    if (!strcmp("echo", argv[0])) {
        buildin_echo(argc, argv);
        return;
    }
    if (!strcmp("cat", argv[0])) {
        buildin_cat(argc, argv);
        return;
    }
    if (!strcmp("rm", argv[0])) {
        buildin_rm(argc, argv);
        return;
    }
    if (!strcmp("ls", argv[0])) {
        buildin_ls(argc, argv);
        return;
    }
    if (!strcmp("mkdir", argv[0])) {
        buildin_mkdir(argc, argv);
        return;
    }
    if (!strcmp("rmdir", argv[0])) {
        buildin_rmdir(argc, argv);
        return;
    }
    if (!strcmp("pwd", argv[0])) {
        buildin_pwd(argc, argv);
        return;
    }
    if (!strcmp("cd", argv[0])) {
        buildin_cd(argc, argv);
        return;
    }

    // get absolute path
    char abs[PATH_LEN_MAX];
    if (-1 == path_abs(argv[0], abs)) {
        return;
    }
    strcpy(argv[0], abs);
    pr_debug("%s:abs=%s\n", __func__, abs);

    // check if file exist
    struct fstat st;
    if (-1 == stat(argv[0], &st)) {
        printf("%s:No such file or directory\n", __func__);
        return;
    }

    // execute command
    int16_t pid = fork();
    if (pid) { // parent
        int32_t status;
        int16_t pid = wait(&status);
        printf("exit status:%d\n", status);
    } else { // child
        exec(argv[0], argv);
    }
}

static void cmd_pipe(uint8_t* cmd)
{
    pr_debug("%s:cmd=%s\n", __func__, cmd);

    // argument
    int32_t argc = -1;
    uint32_t argv_size = ARG_NR_MAX * sizeof(char*);
    char** argv= malloc(argv_size);

    // pipe: redirect stdout
    int32_t pipefd[2] = {-1};
    pipe(pipefd);
    dup2(stdout_no, pipefd[1]);

    // first command
    char buf[CMD_LEN_MAX];
    strcpy(buf, cmd);
    char * each = buf;
    char* p = strchr(each, '|');
    *p = 0;
    argc = cmd_parse(each, argv);
    cmd_exec(argc, argv);

    // pipe: redirect stdin
    dup2(stdin_no, pipefd[0]);

    // next command
    each = p + 1;
    while (p = strchr(each, '|'))
    {
        *p = 0;
        argc = cmd_parse(each, argv);
        cmd_exec(argc, argv);
        each = p + 1;
    }

    // pipe: recover stdout
    dup2(stdout_no, stdout_no);
    // last command
    argc = cmd_parse(each, argv);
    cmd_exec(argc, argv);

    // pipe: recover stdin
    dup2(stdin_no, stdin_no);

    close(pipefd[0]);
    close(pipefd[1]);
}

//=========================
// external functions
//=========================
void myshell()
{
    int32_t argc = -1;
    uint8_t cmd[CMD_LEN_MAX];
    uint8_t cwd[PATH_LEN_MAX];

    // argument
    uint32_t argv_size = ARG_NR_MAX * sizeof(char*);
    char** argv= malloc(argv_size);

    // enter shell
    printf("Press Any Key to enter shell...");
    read(0, cmd, 1);
    clear();

    while (1)
    {
        getcwd(cwd, PATH_LEN_MAX);
        printf("srhuang@localhost:%s$", cwd);

        // read cmd
        memset(cmd, 0, CMD_LEN_MAX);
        readline(cmd, CMD_LEN_MAX);
        pr_debug("%s:cmd=%s\n", __func__, cmd);

        // check pipe
        if (strchr(cmd, '|')) {
            cmd_pipe(cmd);
            continue;
        }

        // parse cmd
        argc = cmd_parse(cmd, argv);
        pr_debug("%s:argc=%d\n", __func__, argc);
        if (0 == argc) {
            continue;
        }

        // execute cmd
        pr_debug("%s:argv[0]=%s\n", __func__, argv[0]);
        cmd_exec(argc, argv);
        // free argv[]
        uint32_t idx;
        for (idx = 0; idx < argc; idx++)
        {
            free(argv[idx]);
            argv[idx] = NULL;
        }
    }
}


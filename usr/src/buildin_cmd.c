#include "buildin_cmd.h"
#include "printf.h"
#include "syscall_usr.h"
#include "path.h"
#include "string.h"
#include "stddef.h"
#include "stdio.h"

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

//=========================
// external functions
//=========================
void buildin_clear(uint32_t argc, char** argv)
{
    if (argc != 1) {
        printf("clear: no argument support.\n");
        return;
    }
    clear();
}

void buildin_ps(uint32_t argc, char** argv)
{
    if (argc != 1) {
        printf("ps: no argument support.\n");
        return;
    }
    ps();
}

void buildin_echo(uint32_t argc, char** argv)
{
    // echo to stdout
    if (argc == 2) {
        printf("%s\n", argv[1]);
        return;
    }

    // echo to file
    if (argc != 4) {
        printf("echo: message > file.(overwirte)\n");
        printf("echo: message >> file.(append)\n");
        return;
    }


    char flag = 0;
    if (!strcmp(argv[2], ">")) {
        flag = O_WRONLY | O_TRUNC;
    } else if (!strcmp(argv[2], ">>")) {
        flag = O_WRONLY | O_APPEND;
    } else {
        printf("echo: oprand(%s) error\n", argv[2]);
        return;
    }

    char path[PATH_LEN_MAX];
    path_abs(argv[3], path);
    struct fstat st;
    if ((-1 == stat(path, &st)) && (!strcmp(argv[2], ">"))) {
        flag += O_CREATE;
    }
    int fd = open(path, flag);
    if (fd == -1) {
        printf("echo: open: open %s failed\n", argv[3]);
        return;
    }

    write(fd, argv[1], strlen(argv[1]));

    close(fd);
}

void buildin_cat(uint32_t argc, char** argv)
{
    char buf[256] = {0};

    // cat from stdin
    if (argc == 1) {
        int32_t ret= 0;
        char* p = buf;
        read(stdin_no, p, 1);
        write(stdout_no, p, 1);
        while ((*p != '\r') && (*p != '\n') && (*p != 0))
        {
            p++;
            read(stdin_no, p, 1);
            write(stdout_no, p, 1);
        }
        *(p + 1) = 0;
        write(stdout_no, buf, strlen(buf));
        return;
    }

    if (argc != 2) {
        printf("cat: only support 1 argument!\n");
        return;
    }

    //cat from file
    char path[PATH_LEN_MAX];
    path_abs(argv[1], path);
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("echo: open: open %s failed\n", argv[1]);
        return;
    }

    struct fstat st;
    stat(path, &st);
    memset(buf, 0, 256);
    uint32_t idx;
    for (idx = 0; idx < st.size; idx++)
    {
        read(fd, buf, 1);
        write(stdout_no, buf, 1);
    }

    printf("\n");
    close(fd);
}

void buildin_rm(uint32_t argc, char** argv)
{
    if (argc != 2) {
        printf("rm: only support 1 argument!\n");
        return;
    }

    char path[PATH_LEN_MAX];
    path_abs(argv[1], path);
    if (!strcmp(path, "/")) {
        printf("rm: Can not remove root dir\n");
        return;
    }
    if (-1 == unlink(path)) {
        printf("rm: delete %s fail.\n", argv[1]);
    }
}

void buildin_ls(uint32_t argc, char** argv)
{
    if (argc != 1) {
        printf("ls: no argument support.\n");
        return;
    }

    char path[PATH_LEN_MAX];
    if (-1 == getcwd(path, PATH_LEN_MAX)) {
        printf("ls: get work directory failed\n");
        return;
    }
    pr_debug("%s:path=%s\n", __func__, path);

    struct fstat st;
    char ftype;
    struct dirstream* dir = opendir(path);
    struct dirent* dir_e = NULL;
    char tmp[PATH_LEN_MAX];
    printf("type / inode / size / name\n");
    while ((dir_e = readdir(dir)) != NULL)
    {
        // type
        if (dir_e->f_type == FT_FILE) {
            ftype = '-';
        } else if (dir_e->f_type == FT_DIR) {
            ftype = 'd';
        } else {
            ftype = 'u';
        }

        // size
        strcpy(tmp, path);
        strcat(tmp, dir_e->filename);
        pr_debug("%s:tmp=%s\n", __func__, tmp);
        if (stat(tmp, &st) == -1) {
            printf("ls: cannot access %s\n", dir_e->filename);
            return;
        }
        printf("%c\t%d\t%d\t%s\n", \
            ftype, dir_e->i_no, st.size, dir_e->filename);

    } // while
}

void buildin_mkdir(uint32_t argc, char** argv)
{
    if (argc != 2) {
        printf("mkdir: only support 1 argument!\n");
        return;
    }

    char path[PATH_LEN_MAX];
    path_abs(argv[1], path);
    if (!strcmp(path, "/")) {
        printf("mkdir: Cannot create a directory under the root dir\n");
        return;
    }
    if (-1 == mkdir(path)) {
        printf("mkdir: create %s fail.\n", argv[1]);
    }
}

void buildin_rmdir(uint32_t argc, char** argv)
{
    if (argc != 2) {
        printf("rmdir: only support 1 argument!\n");
        return;
    }

    char path[PATH_LEN_MAX];
    path_abs(argv[1], path);
    if (!strcmp(path, "/")) {
        printf("rmdir: Can not remove root dir\n");
        return;
    }
    if (-1 == rmdir(path)) {
        printf("rmdir: delete %s fail.\n", argv[1]);
    }
}

void buildin_pwd(uint32_t argc, char** argv)
{
    if (argc != 1) {
        printf("pwd: no argument support.\n");
        return;
    }

    char path[PATH_LEN_MAX];
    if (-1 == getcwd(path, PATH_LEN_MAX)) {
        printf("pwd: get work directory failed\n");
        return;
    }

    printf("%s\n", path);
}

void buildin_cd(uint32_t argc, char** argv)
{
    if (argc > 2) {
        printf("cd: too many arguments\n");
        return;
    }

    char path[PATH_LEN_MAX];
    if (1 == argc) {
        path[0] = '/';
        path[1] = 0;
    } else {
        path_abs(argv[1], path);
    }

    if (-1 == chdir(path)) {
        printf("cd: no such directory %s\n", path);
        return;
    }
}



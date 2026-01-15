#include "path.h"
#include "printf.h"
#include "syscall_usr.h"
#include "string.h"
#include "stddef.h"

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
int32_t path_abs(char* path, char abs[PATH_LEN_MAX])
{
    char tmp[PATH_LEN_MAX];
    memset(tmp, 0, PATH_LEN_MAX);

    // check path valid
    if (0 == strlen(path)) {
        printf("%s: path is NULL\n", __func__);
        return -1;
    }

    // get absolute path
    if (path[0] != '/') {
        int32_t ret = -1;
        ret = getcwd(tmp, PATH_LEN_MAX);
        if (-1 == ret) {
            return -1;
        }
        pr_debug("%s:getcwd=%s\n", __func__, tmp);
    }
    strcat(tmp, path);
    pr_debug("%s:tmp=%s\n", __func__, tmp);

    // deal with '.' and '..'
    uint8_t* buf = tmp;
    uint8_t* p;
    memset(abs, 0, PATH_LEN_MAX);
    while (0 != *buf)
    {
        p = strchr(buf, '/');
        if (NULL == p) { // the last name
            strcat(abs, buf);
            break;
        } else {
            *p = 0;
            if (!strcmp(buf, "..")) {
                char* parent_dir = strrchr(abs, '/');
                if (parent_dir != abs) {
                    //not return to the root dir
                    *parent_dir = 0;
                    parent_dir = strrchr(abs, '/');
                    *(parent_dir + 1) = 0;
                }
            } else if (!strcmp(buf, ".")) {
                // do nothing
            } else {
                strcat(abs, buf);
                strcat(abs, "/");
            } // if (!strcmp(buf, ".."))
            buf = p + 1;
         } // if (NULL == p)
    } // while

    pr_debug("%s:abs=%s\n", __func__, abs);
    return 0;
}


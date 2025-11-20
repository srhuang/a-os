#include "dir.h"
#include "printk.h"
#include "stddef.h"
#include "ide.h"
#include "memory.h"
#include "inode.h"
#include "string.h"

//=========================
// debugging
//=========================
//#define DEBUG

#ifdef DEBUG
    #define pr_debug(fmt, ...) printk(fmt, ##__VA_ARGS__)
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
struct ide_ptn* dir_get_ptn(const char* name)
{
    struct ide_ptn* ret_ptn = NULL;

    // get root dir
    struct dirent* dir = (struct dirent*)root_dir.inode->i_block[0];

    // parsing the root dir
    int dir_idx=0;
    while (dir_idx < ROOT_DIR_MAX)
    {
        if (FT_UNKNOWN != (dir + dir_idx)->f_type) {
            if (!strcmp((dir + dir_idx)->filename, name)) {
                ret_ptn = inode_get_ptn((dir + dir_idx)->i_no);
                break;
            }
        }
        dir_idx++;
    } // while

    if (NULL == ret_ptn) {
        printk("%s:Invalid dir name:%s\n", __func__, name);
    }

    return ret_ptn;
}

int32_t dir_split_path(const char* path, \
    char name[PATH_DEPTH_MAX][FILE_NAME_MAX])
{
    uint8_t* buf = (uint8_t*)sys_malloc(strlen(path));
    memcpy(buf, path, strlen(path));

    //pr_debug("%s:size=%d\n", __func__, strlen(path));

    // check path valid
    if (0 == strlen(path)) {
        printk("%s: path is NULL\n", __func__);
        return -1;
    }

    // check the first char
    if ('/' != *buf) {
        printk("Only support absolute path\n");
        printk("%s:path=%s\n", __func__, path);
        return -1;
    }
    buf++;

    uint8_t* p;
    uint32_t idx = 0;
    while (0 != *buf)
    {
        if (idx > PATH_DEPTH_MAX) {
            printk("%s:path depth is over size.(%d)\n", __func__, idx);
            return -1;
        }
        p = strchr(buf, '/');
        memset(name[idx], 0, FILE_NAME_MAX);
        if (NULL == p) { // the last name
            strcpy(name[idx++], buf);
            break;
        } else {
            memcpy(name[idx++], buf, p-buf);
            buf = p + 1;
        }
    } // while

    return idx;
}

//=========================
// external functions
//=========================
int dir_search_name(uint32_t inode_no, const char* name, enum file_types type)
{
    int ret_inode = -1;

    //pr_debug("%s:inode:%d, name:%s\n", __func__, inode_no, name);

    // get the inode
    struct inode_sys* inode;
    inode = inode_open(inode_no);
    if (NULL == inode) {
        printk("%s:Invalid inode number:%d\n", __func__, inode_no);
        return -1;
    }

    // parsing the inode block
    void* buf = sys_malloc(inode->i_size);
    struct dirent* dir = (struct dirent*)buf;
    uint32_t dir_idx_max = inode->i_size / sizeof(struct dirent);

    // read from hard disk
    inode_read(inode, 0, buf, inode->i_size);

    // find name
    uint32_t dir_idx;
    for (dir_idx=0; dir_idx < dir_idx_max; dir_idx++)
    {
        if (!strcmp((dir + dir_idx)->filename, name) &&
            ((dir + dir_idx)->f_type == type))
        {
            ret_inode = (dir + dir_idx)->i_no;
            // transfer the inode number
            struct ide_ptn* ptn = inode_get_ptn(inode_no);
            ret_inode += ptn->inode_base;
            break;
        }
    } // for

    sys_free(buf);
    inode_close(inode);

    return ret_inode;
}

int32_t dir_search_path(struct dirstream* dir, const char* path)
{
    //const char* p_path = path;
    char name[PATH_DEPTH_MAX][FILE_NAME_MAX];

    //pr_debug("%s:%s\n", __func__, path);

    // root dir
    if (!strcmp(path, "/") \
        || !strcmp(path, "/.") \
        || !strcmp(path, "/..")) {
        return 0; //root dir
    }

    int32_t name_cnt = dir_split_path(path, name);
    if (-1 == name_cnt) {
        return -1;
    }

    // get partition
    struct ide_ptn* ptn;
    ptn = dir_get_ptn(name[0]);
    if (NULL == ptn) {
        return -1;
    }
    dir->path[0] = ptn->inode_base;

    // parsing the rest of path
    int32_t i_no;
    uint32_t idx = 1;
    while (idx < name_cnt)
    {
        // searching with name
        i_no = dir_search_name(dir->path[idx-1], name[idx], FT_DIR);
        if (-1 == i_no) {
            printk("Can NOT find the dir %s.\n", name[idx]);
            return -1;
        }
        dir->path[idx] = i_no;
        //pr_debug("%s:dir->path[%d]=%d\n", __func__, idx, i_no);

        idx++;
    } // while

    return idx;
}

int32_t dir_parse_path(const char* path, char* child_name)
{
    int32_t i_parent = -1;

    // get parent inode and child name
    struct dirstream dir;
    uint32_t size = strlen(path);
    uint8_t* buf = (uint8_t*)sys_malloc(size);
    strcpy(buf, path);
    if (*(buf+size-1) == '/') {
        *(buf+size-1) = 0;
    }
    uint8_t* p = strrchr(buf, '/');
    *p = 0;
    strcpy(child_name, p + 1);
    int32_t cnt = dir_search_path(&dir, buf);
    if (-1 == cnt) {
        return -1;
    }
    i_parent = dir.path[cnt-1];
    sys_free(buf);

    return i_parent;
}

void dir_install(uint32_t i_parent, struct dirent* new_dir)
{
    // install new dir to parent inode table
    struct inode_sys* inode = inode_open(i_parent);
    inode_write(inode, inode->i_size, (uint8_t*)new_dir, sizeof(struct dirent));
    inode_close(inode);
}

void dir_uninstall(uint32_t i_parent, uint32_t i_child)
{
    // update parent inode
    struct inode_sys* inode = inode_open(i_parent);
    struct ide_ptn* ptn = inode_get_ptn(i_parent);
    void* buf = sys_malloc(inode->i_size);
    inode_read(inode, 0, buf, inode->i_size);
    struct dirent* dir_entry = (struct dirent*)buf;
    uint32_t idx_max = inode->i_size / sizeof(struct dirent);
    uint32_t i_child_hd = i_child - ptn->inode_base;
    uint32_t idx = 0;
    while (idx < idx_max)
    {
        if (((dir_entry + idx)->i_no) == i_child_hd) {
            // replace it with the last dir
            memcpy(dir_entry + idx, dir_entry + idx_max - 1, \
                sizeof(struct dirent));
            // erase the last dir entry
            inode_erase(inode, sizeof(struct dirent));
            // update data block
            inode_write(inode, 0, buf, inode->i_size);
            break;
        }
        idx++;
    }
    sys_free(buf);
    inode_close(inode);
}

struct dirstream* sys_opendir(const char* path)
{
    struct dirstream* dir = \
        (struct dirstream*)sys_malloc(sizeof(struct dirstream));
    uint32_t i_no;

    int32_t cnt = dir_search_path(dir, path);
    if (-1 == cnt) {
        sys_free(dir);
        return NULL;
    }

    if (0 == cnt) {
        // root dir
        dir->inode = root_dir.inode;
    } else {
        dir->inode = inode_open(dir->path[cnt-1]);
    }
    dir->pos = 0;

    return dir;
}

void sys_closedir(struct dirstream* dir)
{
    if (0 != dir->inode->i_no) {
        inode_close(dir->inode);
    }
    sys_free(dir);
}

void sys_rewinddir(struct dirstream* dir)
{
    dir->pos = 0;
}

struct dirent* sys_readdir(struct dirstream* dir)
{
    int32_t ret = -1;

    // check inode size
    if (dir->pos >= dir->inode->i_size) {
        return NULL;
    }

    struct ide_ptn* ptn = inode_get_ptn(dir->inode->i_no);

    // get the next dirent
    uint32_t step = sizeof(struct dirent);
    uint32_t idx = dir->pos / step;
    if (dir->inode == root_dir.inode) { // root dir
        if (idx >= ROOT_DIR_MAX) {
            return NULL;
        } else {
            memcpy((uint8_t*)&dir->dir_entry, \
                &root_blk[idx], step);
        }
    } else { // hard disk dir
        ret = inode_read(dir->inode, \
            dir->pos, (uint8_t*)&dir->dir_entry, step);
        if (-1 == ret) {
            return NULL;
        }
        // transfer the inode number
        dir->dir_entry.i_no += ptn->inode_base;
    }
    // check valid
    if (FT_UNKNOWN == dir->dir_entry.f_type) {
        return NULL;
    }

    // success
    dir->pos += step;
    return &dir->dir_entry;
}

int32_t sys_mkdir(const char* path)
{
    // get parent inode and child name
    uint8_t child_name[FILE_NAME_MAX];
    uint32_t i_parent = dir_parse_path(path, child_name);

    // new inode table
    struct ide_ptn* ptn = inode_get_ptn(i_parent);
    if (NULL == ptn) {
        return -1;
    }
    struct inode_sys* inode = inode_acquire(ptn);
    // transfer the inode number
    uint32_t i_no_hd = inode->i_no - ptn->inode_base;

    // new block data
    struct dirent new_dir[2];
    memset(new_dir, 0, sizeof(struct dirent)*2);
    // init '.'
    memcpy(&new_dir[0].filename, ".", 1);
    new_dir[0].i_no = i_no_hd;
    new_dir[0].f_type = FT_DIR;
    // init '..'
    memcpy(new_dir[1].filename, "..", 2);
    new_dir[1].i_no = i_parent - ptn->inode_base;
    new_dir[1].f_type = FT_DIR;
    // write to inode
    inode_write(inode, 0, (uint8_t*)new_dir, sizeof(struct dirent)*2);
    inode_close(inode);

    // install into parent
    memset(new_dir, 0, sizeof(struct dirent));
    memcpy(&new_dir[0].filename, child_name, strlen(child_name));
    new_dir[0].i_no = i_no_hd;
    new_dir[0].f_type = FT_DIR;
    dir_install(i_parent, &new_dir[0]);

    return 0;
}

int32_t sys_rmdir(const char* path)
{
    int32_t i_child = -1;
    int32_t i_parent = -1;
    struct inode_sys* inode;

    // get child and parent inode
    struct dirstream dir;
    int32_t cnt = dir_search_path(&dir, path);
    if (-1 == cnt) {
        return -1;
    }
    i_child = dir.path[cnt - 1];
    i_parent = dir.path[cnt - 2];
    pr_debug("%s:i_child=%d\n", __func__, i_child);
    pr_debug("%s:i_parent=%d\n", __func__, i_parent);

    // release child inode
    inode = inode_open(i_child);
    if (inode->i_size > sizeof(struct dirent)*2) {
        printk("%s:dir is NOT empty.(size=%d)\n", __func__, inode->i_size);
        inode_close(inode);
        return -1;
    } else {
        inode_release(inode);
    }

    // uninstall from parent
    dir_uninstall(i_parent, i_child);

    return 0;
}


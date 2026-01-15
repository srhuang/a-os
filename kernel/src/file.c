#include "file.h"
#include "dir.h"
#include "string.h"
#include "memory.h"
#include "printk.h"
#include "stddef.h"
#include "inode.h"
#include "thread.h"
#include "lock.h"
#include "stdio.h"
#include "ioqueue.h"
#include "fcntl.h"
#include "keyboard.h"

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
struct file fd_table[MAX_FILE_OPEN];
struct mutex fd_table_lock;

//=========================
// internal functions
//=========================
static int32_t fd_acquire(struct inode_sys* inode, uint8_t flag)
{
    int32_t ret = -1;
    mutex_lock(&fd_table_lock);

    // find free fd table
    uint32_t fd_idx = 3; // 0, 1, 2 for stdin, stdout, stderr
    while (fd_idx < MAX_FILE_OPEN)
    {
        if (fd_table[fd_idx].inode == NULL) {
            break;
        }
        fd_idx++;
    }
    if (MAX_FILE_OPEN == fd_idx) {
        printk("%s:system fd table is full.\n", __func__);
        goto out;
    }

    // set fd table
    fd_table[fd_idx].inode = inode;
    fd_table[fd_idx].pos = 0;
    fd_table[fd_idx].flag = flag;

    //success
    ret = fd_idx;

out:
    mutex_unlock(&fd_table_lock);
    return ret;
}

static void fd_release(uint32_t fd_idx)
{
    mutex_lock(&fd_table_lock);

    fd_table[fd_idx].inode = NULL;

    mutex_unlock(&fd_table_lock);
}

static int32_t task_install(uint32_t fd_idx)
{
    // install to thread PCB
    struct task_struct* task = kthread_current();
    uint32_t task_fd_idx = 3; // 0, 1, 2 for stdin, stdout, stderr
    while (task_fd_idx < MAX_FD_PER_TASK)
    {
        if (-1 == task->open_fd[task_fd_idx]) {
            task->open_fd[task_fd_idx] = fd_idx;
            break;
        }
        task_fd_idx++;
    }
    if (MAX_FD_PER_TASK == task_fd_idx) {
        printk("%s:task %s fd table is full.\n", __func__, task->name);
        return -1;
    }

    return task_fd_idx;
}

static void task_uninstall(uint32_t task_fd_idx)
{
    struct task_struct* task = kthread_current();
    task->open_fd[task_fd_idx] = -1;
}

static int32_t file_create(uint32_t i_parent, char* name)
{
    int32_t i_no;
    uint32_t i_no_hd;

    // new inode table
    struct ide_ptn* ptn = inode_get_ptn(i_parent);
    if (NULL == ptn) {
        printk("%s:Can NOT get partition for inode(%d)\n", __func__, i_parent);
        return -1;
    }
    struct inode_sys* inode = inode_acquire(ptn);
    i_no = inode->i_no;
    // transfer the inode number
    i_no_hd = inode->i_no - ptn->inode_base;
    inode_close(inode);

    // install to parent inode table
    struct dirent new_dir;
    memset(&new_dir, 0, sizeof(struct dirent));
    memcpy(&new_dir.filename, name, strlen(name));
    new_dir.i_no = i_no_hd;
    new_dir.f_type = FT_FILE;
    dir_install(i_parent, &new_dir);

    return i_no;
}

static int32_t pipe_read(int32_t fd, void* dst, uint32_t cnt)
{
    uint8_t* p_dst = (uint8_t*)dst;
    struct ioqueue* ioq = (struct ioqueue*)fd_table[fd].inode;
    uint32_t len = ioq_len(ioq);
    uint32_t size = (len < cnt ? len : cnt);

    uint32_t idx;
    for (idx = 0; idx < size; idx++)
    {
        *p_dst++ = ioq_get(ioq);
    }

    return idx;
}

static int32_t pipe_write(int32_t fd, const void* src, uint32_t cnt)
{
    uint8_t* p_src = (uint8_t*)src;
    struct ioqueue* ioq = (struct ioqueue*)fd_table[fd].inode;
    uint32_t slot = ioq->size - ioq_len(ioq);
    uint32_t size = (slot < cnt ? slot : cnt);

    uint32_t idx;
    for (idx = 0; idx < size; idx++)
    {
        ioq_put(ioq, *p_src++);
    }

    return idx;
}

//=========================
// external functions
//=========================
int32_t sys_open(const char* path, uint8_t flag)
{
    // get parent inode and child name
    uint8_t child_name[FILE_NAME_MAX];
    int32_t i_parent = dir_parse_path(path, child_name);
     if (-1 == i_parent) {
        return -1;
    }

    // search child name
    int32_t i_no = dir_search_name(i_parent, child_name, FT_FILE);

    // check valid
    if ((-1 == i_no) && !(flag & O_CREATE)) {
        printk("%s: file %s is NOT exist.\n", __func__, child_name);
        return -1;
    }
    if ((-1 != i_no) && (flag & O_CREATE)) {
        printk("%s: file %s has already exist.\n", __func__, child_name);
        return -1;
    }

    // create new inode
    if ((-1 == i_no) && (flag & O_CREATE)) {
        i_no = file_create(i_parent, child_name);
        if (-1 == i_no) {
            return -1;
        }
        pr_debug("%s:After create, i_no=%d\n", __func__, i_no);
    }

    // opne inode
    struct inode_sys* inode = inode_open(i_no);
    if (NULL == inode) {
        printk("%s:Invalid inode(%d).\n", __func__, i_no);
        return -1;
    }

    // acquire fd
    int32_t fd_idx = fd_acquire(inode, flag);
    if (-1 == fd_idx) {
        return -1;
    }

    // install into task
    int32_t task_fd_idx = task_install(fd_idx);
    if (-1 == task_fd_idx) {
        return -1;
    }

    return task_fd_idx;
}

void sys_close(uint32_t task_fd_idx)
{
    struct task_struct* task = kthread_current();
    uint32_t fd_idx = task->open_fd[task_fd_idx];
    struct inode_sys* inode = fd_table[fd_idx].inode;

    // deal with the fd_table
    if (fd_table[fd_idx].flag & O_PIPE) {
        // pipe fd
        fd_table[fd_idx].pos--;
        if (0 == fd_table[fd_idx].pos) {
            ioq_deinit((struct ioqueue*)fd_table[fd_idx].inode);
            fd_release(fd_idx);
        }
    } else {
        // close inode
        inode_close(inode);
        // release fd
        fd_release(fd_idx);
    }

    // uninstall from task
    task_uninstall(task_fd_idx);
}

int32_t sys_unlink(const char* path)
{
    // get parent inode and child name
    uint8_t child_name[FILE_NAME_MAX];
    int32_t i_parent = dir_parse_path(path, child_name);
     if (-1 == i_parent) {
        return -1;
    }

    // search child name
    int32_t i_child = dir_search_name(i_parent, child_name, FT_FILE);
    if (-1 == i_child) {
        printk("%s:Can NOT find file %s.\n", __func__, child_name);
        return -1;
    }

    // search FD table
    uint32_t fd_idx = 3; // 0, 1, 2 for stdin, stdout, stderr
    while (fd_idx < MAX_FILE_OPEN)
    {
        if (fd_table[fd_idx].inode == NULL) {
            fd_idx++;
            continue;
        }
        if (i_child = fd_table[fd_idx].inode->i_no) {
            printk("%s:%s is in use, not allow to delete.\n", \
                __func__, child_name);
            return -1;
        }
        fd_idx++;
    }

    // delete the file
    struct inode_sys* inode = inode_open((uint32_t)i_child);
    inode_release(inode);

    // uninstall from parent
    dir_uninstall(i_parent, i_child);

    return 0;
}

int32_t sys_read(uint32_t task_fd_idx, uint8_t* buf, uint32_t cnt)
{
    struct task_struct* task = kthread_current();
    uint32_t fd_idx = task->open_fd[task_fd_idx];
    struct inode_sys* inode = fd_table[fd_idx].inode;

    // check pipe
    if (fd_table[fd_idx].flag & O_PIPE) {
        return pipe_read(fd_idx, buf, cnt);
    }

    // stdin
    if (stdin_no == task_fd_idx) {
        // keyboard input
        uint32_t idx;
        for (idx = 0; idx < cnt; idx++)
        {
            *buf++ = ioq_get(kb_buf);
        }
        return cnt;
    }

    // file read
    inode_read(inode, fd_table[fd_idx].pos, buf, cnt);
    fd_table[fd_idx].pos += cnt;

    return cnt;
}

int32_t sys_write(uint32_t task_fd_idx, uint8_t* buf, uint32_t cnt)
{
    struct task_struct* task = kthread_current();
    uint32_t fd_idx = task->open_fd[task_fd_idx];
    struct inode_sys* inode = fd_table[fd_idx].inode;

    // check pipe
    if (fd_table[fd_idx].flag & O_PIPE) {
        return pipe_write(fd_idx, buf, cnt);
    }

    // stdout
    if (stdout_no == task_fd_idx) {
        printk("%s", buf);
        return cnt;
    }

    // file write
    if (O_TRUNC & fd_table[fd_idx].flag) {
        inode_write(inode, 0, buf, cnt);
    } else if (O_APPEND & fd_table[fd_idx].flag) {
        inode_write(inode, inode->i_size, buf, cnt);
    } else {
        printk("%s:Undefined flag : 0x%x.\n", __func__, fd_table[fd_idx].flag);
        return -1;
    }

    return cnt;
}

int32_t sys_lseek(int32_t task_fd_idx, int32_t offset, enum whence wh)
{
    struct task_struct* task = kthread_current();
    uint32_t fd_idx = task->open_fd[task_fd_idx];
    struct inode_sys* inode = fd_table[fd_idx].inode;

    switch (wh)
    {
        case SEEK_SET:
            fd_table[fd_idx].pos = offset;
            break;

        case SEEK_CUR:
            fd_table[fd_idx].pos += offset;
            break;

        case SEEK_END:
            fd_table[fd_idx].pos = inode->i_size + offset;
            break;

        default:
            printk("%s:Invalid whence:%d.\n", __func__, wh);
            return -1;
    }

    return fd_table[fd_idx].pos;
}

int32_t sys_stat(const char* path, struct fstat* buf)
{
    // root dir
    if (!strcmp(path, "/") \
        || !strcmp(path, "/.") \
        || !strcmp(path, "/..")) {
        buf->size = root_dir.inode->i_size;
        buf->ftype = FT_DIR;
        return 0;
    }

    // get parent inode and child name
    uint8_t child_name[FILE_NAME_MAX];
    int32_t i_parent = dir_parse_path(path, child_name);
    pr_debug("i_parent=%d, child=%s\n", i_parent, child_name);
    if (-1 == i_parent) {
        return -1;
    }

    //* search child name
    int32_t i_no = dir_search_name(i_parent, child_name, FT_DIR);
    if (-1 == i_no) {
        i_no = dir_search_name(i_parent, child_name, FT_FILE);
        if (-1 == i_no) {
            //printk("%s:%s Not Found.\n", __func__, path);
            return -1;
        }
        // file
        buf->ftype = FT_FILE;
    } else {
        // directory
        buf->ftype = FT_DIR;
    }
    pr_debug("i_no=%d\n", i_no);

    // get inode size
    struct inode_sys* inode = inode_open(i_no);
    buf->size = inode->i_size;
    inode_close(inode);
    return 0;
}

int32_t sys_pipe(uint32_t fd[2])
{
    struct ioqueue* ioq = ioq_init(PIPE_IOQ_SIZE);

    // acquire fd
    int32_t fd_idx = fd_acquire((struct inode_sys*)ioq, O_PIPE);
    if (-1 == fd_idx) {
        return -1;
    }

    // Reuse pos as the pipe open count
    fd_table[fd_idx].pos = 2;
    fd[0] = task_install(fd_idx);
    fd[1] = task_install(fd_idx);
    return 0;
}

void sys_dup2(uint32_t oldfd, uint32_t newfd)
{
    struct task_struct* task = kthread_current();

    if (newfd < 3) {
        task->open_fd[oldfd] = newfd;
    } else {
        task->open_fd[oldfd] = task->open_fd[newfd];
    }
}

void file_init()
{
    mutex_init(&fd_table_lock);
    uint32_t idx;
    for (idx = 0; idx < MAX_FILE_OPEN; idx++)
    {
        fd_table[idx].inode = NULL;
    }
}


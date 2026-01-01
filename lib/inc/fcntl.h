#ifndef __LIB_INC_FCNTL_H
#define __LIB_INC_FCNTL_H

//=========================
// define
//=========================
#define FILE_NAME_MAX       (16)
#define PATH_DEPTH_MAX      (16)

// flags
#define O_RDONLY        (0x1 << 0)
#define O_WRONLY        (0x2 << 0)
#define O_RDWR          (0x3 << 0)
#define O_CREATE        (0x1 << 2)
#define O_TRUNC         (0x1 << 3)
#define O_APPEND        (0x1 << 4)
#define O_PIPE          (0x1 << 7)

enum file_types {
    FT_UNKNOWN,
    FT_FILE,
    FT_DIR
};

enum whence {
    SEEK_SET = 1,
    SEEK_CUR,
    SEEK_END
};

//=========================
// struct
//=========================
// entry of directory in hard disk
struct dirent
{
    uint32_t            i_no;
    char                filename[FILE_NAME_MAX];
    enum file_types     f_type;
};

// for the operations of directory in system
struct dirstream
{
    struct inode_sys*   inode;
    uint32_t            path[PATH_DEPTH_MAX];
    struct dirent       dir_entry;
    uint32_t            pos;
};

// file state
struct fstat {
    uint32_t            size;
    enum file_types     ftype;
};
//=========================
// external variable
//=========================

//=========================
// function
//=========================

#endif

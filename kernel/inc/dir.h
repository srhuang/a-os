#ifndef __KERNEL_INC_DIR_H
#define __KERNEL_INC_DIR_H
#include "stdint.h"
#include "fs.h"

//=========================
// define
//=========================

//=========================
// struct
//=========================

//=========================
// external variable
//=========================

//=========================
// function
//=========================
int                 dir_search_name(uint32_t inode_no, const char* name, \
                        enum file_types type);
int32_t             dir_search_path(struct dirstream* dir, const char* path);
int32_t             dir_parse_path(const char* path, char* child_name);
void                dir_install(uint32_t i_parent, struct dirent* new_dir);
void                dir_uninstall(uint32_t i_parent, uint32_t i_child);

// for system call
struct dirstream*   sys_opendir(const char* path);
void                sys_closedir(struct dirstream* dir);
void                sys_rewinddir(struct dirstream* dir);
struct dirent*      sys_readdir(struct dirstream* dir);
int32_t             sys_mkdir(const char* path);
int32_t             sys_rmdir(const char* path);

#endif

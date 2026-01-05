#ifndef __USR_INC_BUILD_CMD_H
#define __USR_INC_BUILD_CMD_H
#include "stdint.h"

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
void    buildin_clear(uint32_t argc, char** argv);
void    buildin_ps(uint32_t argc, char** argv);
void    buildin_echo(uint32_t argc, char** argv);
void    buildin_cat(uint32_t argc, char** argv);
void    buildin_rm(uint32_t argc, char** argv);
void    buildin_ls(uint32_t argc, char** argv);
void    buildin_mkdir(uint32_t argc, char** argv);
void    buildin_rmdir(uint32_t argc, char** argv);
void    buildin_pwd(uint32_t argc, char** argv);
void    buildin_cd(uint32_t argc, char** argv);

#endif

#ifndef __USR_INC_SHELL_H
#define __USR_INC_SHELL_H

//=========================
// define
//=========================
#define PATH_LEN_MAX        (256)
#define ARG_NR_MAX          (16)
#define CMD_LEN_MAX         (512)

#define KB_CTRL_L           (0x0C)
#define KB_CTRL_U           (0x15)

//=========================
// struct
//=========================

//=========================
// external variable
//=========================

//=========================
// function
//=========================
void myshell(void);

#endif

#ifndef __USR_INC_PATH_H
#define __USR_INC_PATH_H
#include "stdint.h"

//=========================
// define
//=========================
#define PATH_LEN_MAX        (256)

//=========================
// struct
//=========================

//=========================
// external variable
//=========================

//=========================
// function
//=========================
int32_t path_abs(char* path, char abs[PATH_LEN_MAX]);

#endif

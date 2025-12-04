#ifndef __KERNEL_INC_PROCESS_H
#define __KERNEL_INC_PROCESS_H

//=========================
// define
//=========================
#define USR_START_SECTOR    (205)
#define USR_SECTOR_COUNT    (100)
#define USR_INIT_PATH       "/sdb_1/init"

//=========================
// struct
//=========================

//=========================
// external variable
//=========================

//=========================
// function
//=========================
void process_init(void);

#endif

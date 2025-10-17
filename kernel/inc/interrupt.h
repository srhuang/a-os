#ifndef __KERNEL_INC_INTERRUPT_H
#define _KERNEL_INC_INTERRUPT_H
#include "stdint.h"
#include "print.h"

//=========================
// define
//=========================
#define IDT_VEC_MAX         (0x80)
#define IDT_DESC_CNT        (0x30)

//=========================
// struct
//=========================


//=========================
// external variable
//=========================
extern void* intr_entry[IDT_DESC_CNT];

//=========================
// function
//=========================
void intr_init(void);
void register_handler(uint8_t vector_no, void* function);

#endif

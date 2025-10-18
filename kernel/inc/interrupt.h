#ifndef __KERNEL_INC_INTERRUPT_H
#define _KERNEL_INC_INTERRUPT_H
#include "stdint.h"
#include "print.h"
#include "stdbool.h"

//=========================
// define
//=========================
#define IDT_VEC_MAX         (0x80)
#define IDT_DESC_CNT        (0x30)

// PIC
#define PIC_M_CTRL  (0x20)  // master control
#define PIC_M_DATA  (0x21)  // master data
#define PIC_S_CTRL  (0xA0)  // slave control
#define PIC_S_DATA  (0xA1)  // slave data

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
bool intr_get_status(void);
void intr_set_status (bool intr_status);
void intr_init(void);
void register_handler(uint8_t vector_no, void* function);

#endif

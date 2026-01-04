#ifndef __KERNEL_INC_KEYBOARD_H
#define __KERNEL_INC_KEYBOARD_H
#include "ioqueue.h"

//=========================
// define
//=========================
#define KB_BUF_PORT         (0x60)

// key code
#define KB_EXT_CODE         (0xE0)
#define KB_BREAK_CODE       (0x80)

// Modifier keys
#define KB_SHIFT_L_MAKE     (0x2A)
#define KB_SHIFT_L_BREAK    (0xAA)
#define KB_SHIFT_R_MAKE     (0x36)
#define KB_SHIFT_R_BREAK    (0xB6)
#define KB_CTRL_L_MAKE      (0x1D)
#define KB_CTRL_L_BREAK     (0x9D)
#define KB_CTRL_R_MAKE      (0xE01D)
#define KB_CTRL_R_BREAK     (0xE09D)
#define KB_ALT_L_MAKE       (0x38)
#define KB_ALT_L_BREAK      (0xB8)
#define KB_ALT_R_MAKE       (0xE038)
#define KB_ALT_R_BREAK      (0xE0B8)
#define KB_CAPS_MAKE        (0x3A)

// hot key
#define KB_CTRL_L           (0x0C)
#define KB_CTRL_U           (0x15)

// io queue
#define KB_IOQ_SIZE         (4096)

//=========================
// struct
//=========================

//=========================
// external variable
//=========================
extern struct ioqueue* kb_buf;

//=========================
// function
//=========================
void keyboard_init(void);

#endif

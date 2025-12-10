#ifndef __KERNEL_INC_KERNEL_H
#define __KERNEL_INC_KERNEL_H

//=========================
// GDT Attribute
//=========================
#define GDT_G_1         (0x0 << 23)
#define GDT_G_4K        (0x1 << 23)
#define GDT_D_16        (0x0 << 22)
#define GDT_D_32        (0x1 << 22)
#define GDT_L_32        (0x0 << 21)
#define GDT_L_64        (0x1 << 21)
#define GDT_AVL_0       (0x0 << 20)
#define GDT_AVL_1       (0x1 << 20)
#define GDT_P_0         (0x0 << 15)
#define GDT_P_1         (0x1 << 15)
#define GDT_DPL_0       (0x0 << 13)
#define GDT_DPL_1       (0x1 << 13)
#define GDT_DPL_2       (0x2 << 13)
#define GDT_DPL_3       (0x3 << 13)
#define GDT_S_HW        (0x0 << 12)
#define GDT_S_SW        (0x1 << 12)
#define GDT_TYPE_CODE   (0x8 << 8)
#define GDT_TYPE_DATA   (0x2 << 8)
#define GDT_TYPE_TSS    (0x9 << 8)

//=========================
// GDT Selector Attribute
//=========================
#define TI_GDT      (0x0 << 2)
#define TI_LDT      (0x1 << 2)
#define RPL_0       (0x0)
#define RPL_1       (0x1)
#define RPL_2       (0x2)
#define RPL_3       (0x3)

#define SELECTOR_K_CODE     ((0x1 << 3) + TI_GDT + RPL_0)
#define SELECTOR_K_DATA     ((0x2 << 3) + TI_GDT + RPL_0)
#define SELECTOR_K_STACK    SELECTOR_K_DATA
#define SELECTOR_K_VIDEO    ((0x3 << 3) + TI_GDT + RPL_0)
#define SELECTOR_TSS        ((0x4 << 3) + TI_GDT + RPL_0)
#define SELECTOR_U_CODE     ((0x5 << 3) + TI_GDT + RPL_3)
#define SELECTOR_U_DATA     ((0x6 << 3) + TI_GDT + RPL_3)
#define SELECTOR_U_STACK    SELECTOR_U_DATA

//=========================
// GDT Table
//=========================
#define GDT_BASE            (0xC0000800)
#define GDT_K_CODE          (GDT_BASE + (0x1 << 3))
#define GDT_K_DATA          (GDT_BASE + (0x2 << 3))
#define GDT_K_VIDEO         (GDT_BASE + (0x3 << 3))
#define GDT_TSS             (GDT_BASE + (0x4 << 3))
#define GDT_U_CODE          (GDT_BASE + (0x5 << 3))
#define GDT_U_DATA          (GDT_BASE + (0x6 << 3))

#define GDT_LIMIT           ((0x7 << 3) - 1)
#endif

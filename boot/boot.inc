;-------------------------
; Loader
;-------------------------
LOADER_START_SECTOR     equ 1
LOADER_SECTOR_COUNT     equ 4
LOADER_BASE_ADDR        equ 0x800
LOADER_STACK_TOP        equ LOADER_BASE_ADDR
LOADER_START_ADDR       equ LOADER_BASE_ADDR + 0x200 ; 512 Byte data

;-------------------------
; GDT
;-------------------------
;   %1 : base address
;   %2 : limit
;   %3 : attribute
%macro DESCRIPTOR 3
    dw %2 & 0xffff
    dw %1 & 0xffff
    db (%1 >> 16) & 0xff
    dw ((%2 >> 8) & 0xf00) | ((%3 >> 8) & 0xf0ff)
    db (%1 >> 24) & 0xff
%endmacro

;-------------------------
; GDT attribute
;-------------------------
GDT_G_1         equ (0x0 << 23)
GDT_G_4K        equ (0x1 << 23)
GDT_D_16        equ (0x0 << 22)
GDT_D_32        equ (0x1 << 22)
GDT_L_32        equ (0x0 << 21)
GDT_L_64        equ (0x1 << 21)
GDT_AVL_0       equ (0x0 << 20)
GDT_AVL_1       equ (0x1 << 20)
GDT_P_0         equ (0x0 << 15)
GDT_P_1         equ (0x1 << 15)
GDT_DPL_0       equ (0x0 << 13)
GDT_DPL_1       equ (0x1 << 13)
GDT_DPL_2       equ (0x2 << 13)
GDT_DPL_3       equ (0x3 << 13)
GDT_S_HW        equ (0x0 << 12)
GDT_S_SW        equ (0x1 << 12)
GDT_TYPE_CODE   equ (0x8 << 8)
GDT_TYPE_DATA   equ (0x2 << 8)

;-------------------------
; GDT Selector Attribute
;-------------------------
TI_GDT  equ (0x0 << 2)
TI_LDT  equ (0x1 << 2)
RPL_0   equ (0x0)
RPL_1   equ (0x1)
RPL_2   equ (0x2)
RPL_3   equ (0x3)

;------------------------
; Memory paging
;-------------------------
PAGE_DIR_ADDR   equ 0x0010_0000
PAGE_TABLE_ADDR equ 0x0010_1000

;------------------------
; Memory paging attribute
;------------------------
PG_P        equ (0x1 << 0)
PG_RW_R     equ (0x0 << 1)
PG_RW_W     equ (0x1 << 1)
PG_US_S     equ (0x0 << 2)
PG_US_U     equ (0x1 << 2)
PG_PWT      equ (0x1 << 3)
PG_PCD      equ (0x1 << 4)
PG_A        equ (0x1 << 5)
PG_D        equ (0x1 << 6)
PG_PAT      equ (0x1 << 7)
PG_G        equ (0x1 << 8)
PG_AVL      equ (0x7 << 9)

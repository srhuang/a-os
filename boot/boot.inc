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

%macro DESCRIPTOR_SET_BASE 3
;   %1 : GDT
;   %2 : segment register
;   %3 : base address
    xor eax, eax
    mov ax, %2
    shl eax, 4
    add eax, %3
    mov word [%1 + 2], ax
    shr eax, 16
    mov byte [%1 + 4], al
    mov byte [%1 + 7], ah
%endmacro

%macro CALL_GATE 4
;   %1  : selector
;   %2  : offset
;   %3  : parameter count
;   %4  : attribute
    dw %2 & 0xffff
    dw %1 & 0xffff
    db %3 & 0x1f
    db ((%4 >> 8) & 0xE0) | 0xC
    dw (%2 >> 16) & 0xffff
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
GDT_TYPE_TSS    equ (0x9 << 8)

;-------------------------
; GDT Selector Attribute
;-------------------------
TI_GDT  equ (0x0 << 2)
TI_LDT  equ (0x1 << 2)
RPL_0   equ (0x0)
RPL_1   equ (0x1)
RPL_2   equ (0x2)
RPL_3   equ (0x3)



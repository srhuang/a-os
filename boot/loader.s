%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR

;-------------------------
; GDT
;-------------------------
;                           base,       limit,          attribute
gdt_base:DESCRIPTOR         0x0,        0x0,            0x0
video_desc:DESCRIPTOR       0xb_8000,   0x0_7fff,       GDT_G_1 \
                                                        + GDT_D_32 \
                                                        + GDT_L_32 \
                                                        + GDT_AVL_0 \
                                                        + GDT_P_1 \
                                                        + GDT_DPL_0 \
                                                        + GDT_S_SW \
                                                        + GDT_TYPE_DATA
a_code_desc:DESCRIPTOR      0x0,        A_CODE_LEN-1,   GDT_G_1 \
                                                        + GDT_D_32 \
                                                        + GDT_L_32 \
                                                        + GDT_AVL_0 \
                                                        + GDT_P_1 \
                                                        + GDT_DPL_0 \
                                                        + GDT_S_SW \
                                                        + GDT_TYPE_CODE
a_data_desc:DESCRIPTOR      0x0,        A_DATA_LEN-1,   GDT_G_1 \
                                                        + GDT_D_32 \
                                                        + GDT_L_32 \
                                                        + GDT_AVL_0 \
                                                        + GDT_P_1 \
                                                        + GDT_DPL_0 \
                                                        + GDT_S_SW \
                                                        + GDT_TYPE_DATA
a_stack_desc:DESCRIPTOR     0x0,        A_STACK_LEN-1,  GDT_G_1 \
                                                        + GDT_D_32 \
                                                        + GDT_L_32 \
                                                        + GDT_AVL_0 \
                                                        + GDT_P_1 \
                                                        + GDT_DPL_0 \
                                                        + GDT_S_SW \
                                                        + GDT_TYPE_DATA
ldt_desc:DESCRIPTOR         0x0,        LDT_LEN-1,      GDT_G_1 \
                                                        + GDT_D_32 \
                                                        + GDT_L_32 \
                                                        + GDT_AVL_0 \
                                                        + GDT_P_1 \
                                                        + GDT_DPL_0 \
                                                        + GDT_S_HW \
                                                        + GDT_TYPE_LDT
GDT_SIZE    equ $ - gdt_base
GDT_LIMIT   equ GDT_SIZE - 1

gdtr    dw GDT_LIMIT    ; [15:0] gdt limit
        dd gdt_base     ; [47:16] gdt base

;-------------------------
; LDT
;-------------------------
;                       base,       limit,          attribute
ldt_base:
d_code_desc:DESCRIPTOR  0x0,        D_CODE_LEN-1,   GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_CODE
d_data_desc:DESCRIPTOR  0x0,        D_DATA_LEN-1,   GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_DATA
d_stack_desc:DESCRIPTOR 0x0,        D_STACK_LEN-1,  GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_DATA
LDT_LEN  equ $ - ldt_base
;-------------------------
; GDT selector
;-------------------------
SELECTOR_VIDEO      equ (video_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_A_CODE     equ (a_code_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_A_DATA     equ (a_data_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_A_STACK    equ (a_stack_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_LDT        equ (ldt_desc - gdt_base) + TI_GDT + RPL_0

;-------------------------
; LDT selector
;-------------------------
SELECTOR_D_CODE     equ (d_code_desc - ldt_base) + TI_LDT + RPL_0
SELECTOR_D_DATA     equ (d_data_desc - ldt_base) + TI_LDT + RPL_0
SELECTOR_D_STACK    equ (d_stack_desc - ldt_base) + TI_LDT + RPL_0

;-------------------------
; data
;-------------------------
    message db "2 Loader"

times 512-($-$$) db 0
loader_start:
;-------------------------
; set stack pointer
;-------------------------
    mov sp, LOADER_STACK_TOP

;-------------------------
; get current cursor
;-------------------------
    mov ah, 0x3     ; Function code
    mov bh, 0x0     ; Page Number
    int 0x10        ; BIOS interrupt call
                    ; output: dh=row, dl=column
;-------------------------
; print log
;-------------------------
    mov bp, message ; absolute address of string
    mov ah, 0x13    ; Function code
    mov al, 0x01    ; Write mode
    mov bh, 0x0     ; Page Number
    mov bl, 0x5     ; font color
    mov cx, 0x8     ; length of string
    add dh, 1       ; input: dh=row, dl=column
    mov dl, 0       ; print to next line
    int 0x10        ; BIOS interrupt call

;-------------------------
; set GDT/LDT base address
;-------------------------
;                       GDT/LDT,        sreg,   base address
    DESCRIPTOR_SET_BASE a_code_desc,    cs,     label_a_code
    DESCRIPTOR_SET_BASE a_data_desc,    ds,     label_a_data
    DESCRIPTOR_SET_BASE a_stack_desc,   ss,     label_a_stack
    DESCRIPTOR_SET_BASE ldt_desc,       cs,     ldt_base
    DESCRIPTOR_SET_BASE d_code_desc,    cs,     label_d_code
    DESCRIPTOR_SET_BASE d_data_desc,    ds,     label_d_data
    DESCRIPTOR_SET_BASE d_stack_desc,   ss,     label_d_stack

;-------------------------
; enable protected mode
;-------------------------
; enable A20
    in al, 0x92
    or al, 0000_0010b
    out 0x92, al

; load GDT
    lgdt [gdtr]

; protection enable
    mov eax, cr0
    or eax, 0x0000_0001
    mov cr0, eax

    jmp SELECTOR_A_CODE:0

[bits 32]
;-------------------------
; A data
;-------------------------
label_a_data:
    db "Apple"
A_DATA_LEN  equ $ - label_a_data

;-------------------------
; A stack
;-------------------------
label_a_stack:
    times 256 db 0
A_STACK_LEN equ $ - label_a_stack
A_STACK_TOP equ $ - label_a_stack - 1

;-------------------------
; A code
;-------------------------
label_a_code:
; print log
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    mov edi, (80*17)*2
    mov ax, SELECTOR_A_DATA
    mov ds, ax
    mov bl, [ds:0]
    mov byte [gs:edi], bl
    mov byte [gs:edi+1], 0x4e

    mov ax, SELECTOR_LDT
    lldt ax

    call SELECTOR_D_CODE:0

    jmp $
A_CODE_LEN  equ $ - label_a_code

;-------------------------
; D data
;-------------------------
label_d_data:
    db "Disk"
D_DATA_LEN  equ $ - label_d_data

;-------------------------
; D stack
;-------------------------
label_d_stack:
    times 256 db 0
D_STACK_LEN equ $ - label_d_stack
D_STACK_TOP equ $ - label_d_stack - 1

;-------------------------
; D code
;-------------------------
label_d_code:
; print log
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    mov edi, (80*18)*2
    mov ax, SELECTOR_D_DATA
    mov ds, ax
    mov bl, [ds:0]
    mov byte [gs:edi], bl
    mov byte [gs:edi+1], 0x4e

    retf
D_CODE_LEN  equ $ - label_d_code


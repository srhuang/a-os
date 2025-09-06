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
d_ldt_desc:DESCRIPTOR       0x0,        D_LDT_LEN-1,    GDT_G_1 \
                                                        + GDT_D_32 \
                                                        + GDT_L_32 \
                                                        + GDT_AVL_0 \
                                                        + GDT_P_1 \
                                                        + GDT_DPL_0 \
                                                        + GDT_S_HW \
                                                        + GDT_TYPE_LDT
d_tss_desc:DESCRIPTOR       0x0,        D_TSS_LEN-1,    GDT_G_1 \
                                                        + GDT_D_16 \
                                                        + GDT_L_32 \
                                                        + GDT_AVL_0 \
                                                        + GDT_P_1 \
                                                        + GDT_DPL_0 \
                                                        + GDT_S_HW \
                                                        + GDT_TYPE_TSS
e_ldt_desc:DESCRIPTOR       0x0,        E_LDT_LEN-1,    GDT_G_1 \
                                                        + GDT_D_32 \
                                                        + GDT_L_32 \
                                                        + GDT_AVL_0 \
                                                        + GDT_P_1 \
                                                        + GDT_DPL_0 \
                                                        + GDT_S_HW \
                                                        + GDT_TYPE_LDT
e_tss_desc:DESCRIPTOR       0x0,        E_TSS_LEN-1,    GDT_G_1 \
                                                        + GDT_D_16 \
                                                        + GDT_L_32 \
                                                        + GDT_AVL_0 \
                                                        + GDT_P_1 \
                                                        + GDT_DPL_0 \
                                                        + GDT_S_HW \
                                                        + GDT_TYPE_TSS
; task gate             selector,attribute
d_task_gate:TASK_GATE   SELECTOR_D_TSS, GDT_P_1 + GDT_DPL_0
e_task_gate:TASK_GATE   SELECTOR_E_TSS, GDT_P_1 + GDT_DPL_0

GDT_SIZE    equ $ - gdt_base
GDT_LIMIT   equ GDT_SIZE - 1

gdtr    dw GDT_LIMIT    ; [15:0] gdt limit
        dd gdt_base     ; [47:16] gdt base

;-------------------------
; D LDT
;-------------------------
;                       base,       limit,          attribute
label_d_ldt_base:
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
D_LDT_LEN  equ $ - label_d_ldt_base

;-------------------------
; E LDT
;-------------------------
;                       base,       limit,          attribute
label_e_ldt_base:
e_code_desc:DESCRIPTOR  0x0,        E_CODE_LEN-1,   GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_CODE
e_data_desc:DESCRIPTOR  0x0,        E_DATA_LEN-1,   GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_DATA
e_stack_desc:DESCRIPTOR 0x0,        E_STACK_LEN-1,  GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_DATA
E_LDT_LEN  equ $ - label_e_ldt_base

;-------------------------
; GDT selector
;-------------------------
SELECTOR_VIDEO      equ (video_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_A_CODE     equ (a_code_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_A_DATA     equ (a_data_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_A_STACK    equ (a_stack_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_D_LDT      equ (d_ldt_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_D_TSS      equ (d_tss_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_D_TASK     equ (d_task_gate - gdt_base) + TI_GDT + RPL_0
SELECTOR_E_LDT      equ (e_ldt_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_E_TSS      equ (e_tss_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_E_TASK     equ (e_task_gate - gdt_base) + TI_GDT + RPL_0

;-------------------------
; LDT selector
;-------------------------
SELECTOR_D_CODE     equ (d_code_desc - label_d_ldt_base) + TI_LDT + RPL_0
SELECTOR_D_DATA     equ (d_data_desc - label_d_ldt_base) + TI_LDT + RPL_0
SELECTOR_D_STACK    equ (d_stack_desc - label_d_ldt_base) + TI_LDT + RPL_0
SELECTOR_E_CODE     equ (e_code_desc - label_e_ldt_base) + TI_LDT + RPL_0
SELECTOR_E_DATA     equ (e_data_desc - label_e_ldt_base) + TI_LDT + RPL_0
SELECTOR_E_STACK    equ (e_stack_desc - label_e_ldt_base) + TI_LDT + RPL_0

;-------------------------
; D TSS
;-------------------------
label_d_tss:
    d_tss_backlink      dd 0x0
    d_tss_esp0          dd A_STACK_TOP
    d_tss_ss0           dd SELECTOR_A_STACK
    d_tss_esp1          dd 0x0
    d_tss_ss1           dd 0x0
    d_tss_esp2          dd 0x0
    d_tss_ss2           dd 0x0
    d_tss_cr3           dd 0x0
    d_tss_eip           dd 0x0
    d_tss_eflags        dd 0x0
    d_tss_eax           dd 0x0
    d_tss_ecx           dd 0x0
    d_tss_edx           dd 0x0
    d_tss_ebx           dd 0x0
    d_tss_esp           dd 0x0
    d_tss_ebp           dd 0x0
    d_tss_esi           dd 0x0
    d_tss_edi           dd 0x0
    d_tss_es            dd 0x0
    d_tss_cs            dd SELECTOR_D_CODE
    d_tss_ss            dd SELECTOR_D_STACK
    d_tss_ds            dd SELECTOR_D_DATA
    d_tss_fs            dd 0x0
    d_tss_gs            dd 0x0
    d_tss_ldt           dd SELECTOR_D_LDT
    d_tss_trace         dw 0x0
    d_tss_iobase        dw ($ - label_d_tss +2)
D_TSS_LEN equ $ - label_d_tss

;-------------------------
; E TSS
;-------------------------
label_e_tss:
    e_tss_backlink      dd 0x0
    e_tss_esp0          dd A_STACK_TOP
    e_tss_ss0           dd SELECTOR_A_STACK
    e_tss_esp1          dd 0x0
    e_tss_ss1           dd 0x0
    e_tss_esp2          dd 0x0
    e_tss_ss2           dd 0x0
    e_tss_cr3           dd 0x0
    e_tss_eip           dd 0x0
    e_tss_eflags        dd 0x0
    e_tss_eax           dd 0x0
    e_tss_ecx           dd 0x0
    e_tss_edx           dd 0x0
    e_tss_ebx           dd 0x0
    e_tss_esp           dd 0x0
    e_tss_ebp           dd 0x0
    e_tss_esi           dd 0x0
    e_tss_edi           dd 0x0
    e_tss_es            dd 0x0
    e_tss_cs            dd SELECTOR_E_CODE
    e_tss_ss            dd SELECTOR_E_STACK
    e_tss_ds            dd SELECTOR_E_DATA
    e_tss_fs            dd 0x0
    e_tss_gs            dd 0x0
    e_tss_ldt           dd SELECTOR_E_LDT
    e_tss_trace         dw 0x0
    e_tss_iobase        dw ($ - label_e_tss +2)
E_TSS_LEN equ $ - label_e_tss

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
    DESCRIPTOR_SET_BASE d_code_desc,    cs,     label_d_code
    DESCRIPTOR_SET_BASE d_data_desc,    ds,     label_d_data
    DESCRIPTOR_SET_BASE d_stack_desc,   ss,     label_d_stack
    DESCRIPTOR_SET_BASE d_ldt_desc,     cs,     label_d_ldt_base
    DESCRIPTOR_SET_BASE d_tss_desc,     cs,     label_d_tss
    DESCRIPTOR_SET_BASE e_code_desc,    cs,     label_e_code
    DESCRIPTOR_SET_BASE e_data_desc,    ds,     label_e_data
    DESCRIPTOR_SET_BASE e_stack_desc,   ss,     label_e_stack
    DESCRIPTOR_SET_BASE e_ldt_desc,     cs,     label_e_ldt_base
    DESCRIPTOR_SET_BASE e_tss_desc,     cs,     label_e_tss

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
A_STACK_TOP equ A_STACK_LEN

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

    jmp SELECTOR_D_TASK:0

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
D_STACK_TOP equ D_STACK_LEN

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

    call SELECTOR_E_TASK:0

; print log
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    mov edi, (80*20)*2
    mov ax, SELECTOR_D_DATA
    mov ds, ax
    mov bl, [ds:0]
    mov byte [gs:edi], bl
    mov byte [gs:edi+1], 0x4e

    jmp $

D_CODE_LEN  equ $ - label_d_code

;-------------------------
; E data
;-------------------------
label_e_data:
    db "Egg"
E_DATA_LEN  equ $ - label_e_data

;-------------------------
; E stack
;-------------------------
label_e_stack:
    times 256 db 0
E_STACK_LEN equ $ - label_e_stack
E_STACK_TOP equ E_STACK_LEN

;-------------------------
; E code
;-------------------------
label_e_code:
; print log
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    mov edi, (80*19)*2
    mov ax, SELECTOR_E_DATA
    mov ds, ax
    mov bl, [ds:0]
    mov byte [gs:edi], bl
    mov byte [gs:edi+1], 0x4e
    
    iretd
E_CODE_LEN  equ $ - label_e_code

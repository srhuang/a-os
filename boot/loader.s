%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR

;-------------------------
; GDT
;-------------------------
;                       base,       limit,          attribute
gdt_base:DESCRIPTOR     0x0,        0x0,            0x0
video_desc:DESCRIPTOR   0xb_8000,   0x0_7fff,       GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_3 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_DATA
a_code_desc:DESCRIPTOR  0x0,        A_CODE_LEN-1,   GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_CODE
a_data_desc:DESCRIPTOR  0x0,        A_DATA_LEN-1,   GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_DATA
a_stack_desc:DESCRIPTOR 0x0,        A_STACK_LEN-1,  GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_DATA
b_code_desc:DESCRIPTOR  0x0,        B_CODE_LEN-1,   GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_3 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_CODE
b_data_desc:DESCRIPTOR  0x0,        B_DATA_LEN-1,   GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_3 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_DATA
b_stack_desc:DESCRIPTOR 0x0,        B_STACK_LEN-1,  GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_3 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_DATA
c_code_desc:DESCRIPTOR  0x0,        C_CODE_LEN-1,   GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_CODE
c_data_desc:DESCRIPTOR  0x0,        C_DATA_LEN-1,   GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_DATA
c_stack_desc:DESCRIPTOR 0x0,        C_STACK_LEN-1,  GDT_G_1 \
                                                    + GDT_D_32 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_SW \
                                                    + GDT_TYPE_DATA
tss_desc:DESCRIPTOR     0x0,        TSS_LEN-1,      GDT_G_1 \
                                                    + GDT_D_16 \
                                                    + GDT_L_32 \
                                                    + GDT_AVL_0 \
                                                    + GDT_P_1 \
                                                    + GDT_DPL_0 \
                                                    + GDT_S_HW \
                                                    + GDT_TYPE_TSS

; call gate             selector,offset,parameter,attrbute
c_call_gate:CALL_GATE   SELECTOR_C_CODE, 0x0, 0x0, GDT_P_1 + GDT_DPL_3

GDT_SIZE    equ $ - gdt_base
GDT_LIMIT   equ GDT_SIZE - 1

gdtr    dw GDT_LIMIT    ; [15:0] gdt limit
        dd gdt_base     ; [47:16] gdt base
;-------------------------
; GDT selector
;-------------------------
SELECTOR_VIDEO      equ (video_desc - gdt_base) + TI_GDT + RPL_3
SELECTOR_A_CODE     equ (a_code_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_A_DATA     equ (a_data_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_A_STACK    equ (a_stack_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_B_CODE     equ (b_code_desc - gdt_base) + TI_GDT + RPL_3
SELECTOR_B_DATA     equ (b_data_desc - gdt_base) + TI_GDT + RPL_3
SELECTOR_B_STACK    equ (b_stack_desc - gdt_base) + TI_GDT + RPL_3
SELECTOR_C_CODE     equ (c_code_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_C_DATA     equ (c_data_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_C_STACK    equ (c_stack_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_C_GATE     equ (c_call_gate - gdt_base) + TI_GDT + RPL_3
SELECTOR_TSS        equ (tss_desc - gdt_base) + TI_GDT + RPL_0

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
; set gdt base address
;-------------------------
;                       GDT,            sreg,   base address
    DESCRIPTOR_SET_BASE a_code_desc,    cs,     label_a_code
    DESCRIPTOR_SET_BASE a_data_desc,    ds,     label_a_data
    DESCRIPTOR_SET_BASE a_stack_desc,   ss,     label_a_stack
    DESCRIPTOR_SET_BASE b_code_desc,    cs,     label_b_code
    DESCRIPTOR_SET_BASE b_data_desc,    ds,     label_b_data
    DESCRIPTOR_SET_BASE b_stack_desc,   ss,     label_b_stack
    DESCRIPTOR_SET_BASE c_code_desc,    cs,     label_c_code
    DESCRIPTOR_SET_BASE c_data_desc,    ds,     label_c_data
    DESCRIPTOR_SET_BASE c_stack_desc,   ss,     label_c_stack
    DESCRIPTOR_SET_BASE tss_desc,       cs,     label_tss

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

    mov ax, SELECTOR_TSS
    ltr ax                  ; must execute in CPL=0

    push SELECTOR_B_STACK   ; stack segment
    push B_STACK_TOP        ; top of stack
    push SELECTOR_B_CODE    ; code segment
    push 0                  ; EIP
    retf
A_CODE_LEN  equ $ - label_a_code

;-------------------------
; B data
;-------------------------
label_b_data:
    db "Book"
B_DATA_LEN  equ $ - label_b_data

;-------------------------
; B stack
;-------------------------
label_b_stack:
    times 256 db 0
B_STACK_LEN equ $ - label_b_stack
B_STACK_TOP equ B_STACK_LEN

;-------------------------
; B code
;-------------------------
label_b_code:
; print log
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    mov edi, (80*18)*2
    mov ax, SELECTOR_B_DATA
    mov ds, ax
    mov bl, [ds:0]
    mov byte [gs:edi], bl
    mov byte [gs:edi+1], 0x4e

    push 0x1
    push 0x2
    push 0x3

    call SELECTOR_C_GATE:0
    jmp $           ; stop here!!!
B_CODE_LEN  equ $ - label_b_code

;-------------------------
; C data
;-------------------------
label_c_data:
    db "Cat"
C_DATA_LEN  equ $ - label_c_data

;-------------------------
; C stack
;-------------------------
label_c_stack:
    times 256 db 0
C_STACK_LEN equ $ - label_c_stack
C_STACK_TOP equ C_STACK_LEN

;-------------------------
; C code
;-------------------------
label_c_code:
; print log
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    mov edi, (80*19)*2
    mov ax, SELECTOR_C_DATA
    mov ds, ax
    mov bl, [ds:0]
    mov byte [gs:edi], bl
    mov byte [gs:edi+1], 0x4e

    retf
C_CODE_LEN  equ $ - label_c_code

;-------------------------
; TSS
;-------------------------
label_tss:
    tss_backlink    dd 0x0
    tss_esp0        dd C_STACK_TOP
    tss_ss0         dd SELECTOR_C_STACK
    tss_esp1        dd 0x0
    tss_ss1         dd 0x0
    tss_esp2        dd 0x0
    tss_ss2         dd 0x0
    tss_cr3         dd 0x0
    tss_eip         dd 0x0
    tss_eflags      dd 0x0
    tss_eax         dd 0x0
    tss_ecx         dd 0x0
    tss_edx         dd 0x0
    tss_ebx         dd 0x0
    tss_esp         dd 0x0
    tss_ebp         dd 0x0
    tss_esi         dd 0x0
    tss_edi         dd 0x0
    tss_es          dd 0x0
    tss_cs          dd 0x0
    tss_ss          dd 0x0
    tss_ds          dd 0x0
    tss_fs          dd 0x0
    tss_gs          dd 0x0
    tss_ldt         dd 0x0
    tss_trace       dw 0x0
    tss_iobase      dw ($ - label_tss +2)
TSS_LEN     equ $ - label_tss

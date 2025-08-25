%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR

;-------------------------
; GDT
;-------------------------
;                       base,       limit,      attribute
gdt_base:   DESCRIPTOR  0x0,        0x0,        0x0
code_desc:  DESCRIPTOR  0x0,        0xf_ffff,   GDT_G_4K \
                                                + GDT_D_32 \
                                                + GDT_L_32 \
                                                + GDT_AVL_0 \
                                                + GDT_P_1 \
                                                + GDT_DPL_0 \
                                                + GDT_S_SW \
                                                + GDT_TYPE_CODE
data_desc:  DESCRIPTOR  0x0,        0xf_ffff,   GDT_G_4K \
                                                + GDT_D_32 \
                                                + GDT_L_32 \
                                                + GDT_AVL_0 \
                                                + GDT_P_1 \
                                                + GDT_DPL_0 \
                                                + GDT_S_SW \
                                                + GDT_TYPE_DATA
video_desc: DESCRIPTOR  0xb_8000,   0x0_7fff,   GDT_G_1 \
                                                + GDT_D_32 \
                                                + GDT_L_32 \
                                                + GDT_AVL_0 \
                                                + GDT_P_1 \
                                                + GDT_DPL_0 \
                                                + GDT_S_SW \
                                                + GDT_TYPE_DATA

GDT_SIZE    equ $ - gdt_base
GDT_LIMIT   equ GDT_SIZE - 1
times 256-($-$$) db 0    ; total 32 GDT element

;-------------------------
; fix data location
;-------------------------


;-------------------------
; GDT selector
;-------------------------
SELECTOR_CODE   equ (code_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_DATA   equ (data_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_VIDEO  equ (video_desc - gdt_base) + TI_GDT + RPL_0

;-------------------------
; RAM detection
;-------------------------



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

    jmp SELECTOR_CODE:p_mode_start

;-------------------------
; protected mode start
;-------------------------
[bits 32]
p_mode_start:
   mov ax, SELECTOR_DATA
   mov ds, ax
   mov es, ax
   mov ss, ax
   mov fs, ax
   mov gs, ax
   mov esp,LOADER_STACK_TOP

;-------------------------
; pause the process
;-------------------------
    jmp $           ; stop here!!!

;-------------------------
; non-fix data location
;-------------------------
    message db "2 Loader"
    gdtr    dw GDT_LIMIT    ; [15:0] gdt limit
            dd gdt_base     ; [47:16] gdt base


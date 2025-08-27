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
ram_size    dd 0            ; LOADER_BASE_ADDR + 0x100 = 0x900
ards_nr     dd 0            ; number of ards
ards_buf times 200 db 0     ; total 10 ards

;-------------------------
; GDT selector
;-------------------------
SELECTOR_CODE   equ (code_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_DATA   equ (data_desc - gdt_base) + TI_GDT + RPL_0
SELECTOR_VIDEO  equ (video_desc - gdt_base) + TI_GDT + RPL_0

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
    mov bp, message_1       ; absolute address of string
    mov ah, 0x13            ; Function code
    mov al, 0x01            ; Write mode
    mov bh, 0x0             ; Page Number
    mov bl, 0x5             ; font color
    mov cx, message_1_len   ; length of string
    add dh, 1               ; input: dh=row, dl=column
    mov dl, 0               ; print to next line
    int 0x10                ; BIOS interrupt call

;-------------------------
; get RAM size
;-------------------------
; get all ARDS
get_ram_size:
    xor ebx, ebx            ; for BIOS, must be zero
    mov edx, 0x534d4150     ; Place "SMAP" into edx
    mov di, ards_buf
e820:
    mov eax, 0x0000e820     ; it changes after trigger interrupt
    mov ecx, 20             ; size of ARDS
    int 0x15
    add di, cx              ; point to next buffer
    inc word [ards_nr]      ; increase the number of ARDS
    cmp ebx, 0x0
    jne e820

; find the max address
    mov ebx, ards_buf       ; base address of ARDS
    xor edx, edx            ; for max address
    mov cx, [ards_nr]       ; loop count
find_max_addr_loop:
    mov eax, [ebx]          ; base address low bits
    add eax, [ebx+8]        ; length low bits
    cmp edx, eax            ; compare with max address
    jge next_ards
    mov edx, eax            ; update the max address
next_ards:
    add ebx, 20             ; next ARDS structure
    loop find_max_addr_loop

; set RAM size
    mov [ram_size], edx     ; store the RAM size

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
; get current cursor
;-------------------------
; bx=current cursor
; get high bits of cursor
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    in al, dx
    mov bh, al

; get low bits of cursor
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    in al, dx
    mov bl, al

;-------------------------
; set bx to next line
;-------------------------
; dividend: dx:ax, quotient: ax, remainder: dx
    xor dx, dx      ; clear to zero
    mov ax, bx
    mov si, 80
    div si
    sub bx, dx      ; minus remainder
    add bx, 80      ; next line

;-------------------------
; print log
;-------------------------
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    shl bx, 1               ; 1 char = 2 bytes
    mov edx, message_2
    mov ecx, message_2_len  ; loop count
    xor esi, esi            ; clear to zero
    xor edi, edi            ; clear to zero
putchar_loop:
    mov byte al, [edx+edi]
    mov byte [gs:bx+si], al
    mov byte [gs:bx+si+1], 0x4e
    add di, 1
    add si, 2
    loop putchar_loop

;-------------------------
; set cursor to next line
;-------------------------
    shr bx, 1       ; 2 bytes = 1 char
    add bx, 80      ; next line
; set high bits of cursor
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    mov al, bh
    out dx, al

; set low bits of cursor
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al

;-------------------------
; pause the process
;-------------------------
    jmp $           ; stop here!!!

;-------------------------
; non-fix data location
;-------------------------
    message_1       db "2 Loader"
    message_1_len   equ 8
    message_2       db "3 Protected"
    message_2_len   equ 11
    gdtr    dw GDT_LIMIT    ; [15:0] gdt limit
            dd gdt_base     ; [47:16] gdt base


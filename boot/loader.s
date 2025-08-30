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
; print log
;-------------------------
    mov edi, message_2
    mov ecx, message_2_len
    call print_log

;-------------------------
; enable memory paging
;-------------------------
    call setup_page

; set page directory address
    mov eax, PAGE_DIR_ADDR
    mov cr3, eax

; enable memory paging
    mov eax, cr0
    or eax, 0x8000_0000     ; [31:31] for enabling memory paging
    mov cr0, eax

;-------------------------
; pause the process
;-------------------------
    jmp $           ; stop here!!!

;-------------------------
; print log
;   edi : address of message
;   ecx : length of message
;-------------------------
print_log:
; get current cursor, bx=current cursor
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

; set bx to next line
; dividend: dx:ax, quotient: ax, remainder: dx
    xor dx, dx      ; clear to zero
    mov ax, bx
    mov si, 80
    div si
    sub bx, dx      ; minus remainder
    add bx, 80      ; next line

; print log
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    shl bx, 1               ; 1 char = 2 bytes
    xor edx, edx            ; clear to zero
putchar_loop:
    mov byte al, [edi+edx]
    mov byte [gs:bx], al
    mov byte [gs:bx+1], 0x4e
    add dx, 1
    add bx, 2
    loop putchar_loop

; set cursor
    shr bx, 1               ; 2 bytes = 1 char
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

    ret

;-------------------------
; setup memory paging
;-------------------------
setup_page:
; clear page directory
    mov eax, 0x0
    mov ebx, PAGE_DIR_ADDR
    mov esi, 0                          ; index of entry
    mov ecx, 1024                       ; loop count
clear_page_dir_loop:
    mov [ebx + esi * 4], eax
    inc esi
    loop clear_page_dir_loop

; create page directory entry(PDE)
    mov eax, PAGE_TABLE_ADDR            ; page table 0
    or eax, PG_US_S | PG_RW_W | PG_P    ; only supervisor
    mov esi, 0                          ; index of entry
    mov [ebx + esi * 4], eax            ; for 1 MB kernel
    mov esi, 768                        ; index of entry
    mov [ebx + esi * 4], eax            ; mapping the same 1 MB kernel

; reserve the kernel space, page table 769 - 1022
    add eax, 0x1000
    mov esi, 769                        ; index of entry
    mov ecx, 254                        ; loop count
create_pde_loop:
    mov [ebx + esi * 4], eax
    inc esi
    add eax, 0x1000
    loop create_pde_loop

; last entry of PDE always point to itself
    mov eax, PAGE_DIR_ADDR
    or eax, PG_US_S | PG_RW_W | PG_P    ; only supervisor
    mov esi, 1023                       ; index of entry
    mov [ebx + esi * 4], eax            ; point to itself

; create page table entry (PTE) for page table 0
    mov eax, 0x0                        ; for physical address
    or eax, PG_US_S | PG_RW_W | PG_P
    mov ebx, PAGE_TABLE_ADDR
    mov esi, 0                          ; index of page table
    mov ecx, 256                        ; 4K * 256 = 1 MB
create_pte_loop:
    mov [ebx + esi * 4], eax
    add eax, 0x1000
    inc esi
    loop create_pte_loop

    ret

;-------------------------
; non-fix data location
;-------------------------
    message_1       db "2 Loader"
    message_1_len   equ 8
    message_2       db "3 Protected"
    message_2_len   equ 11
    gdtr    dw GDT_LIMIT    ; [15:0] gdt limit
            dd gdt_base     ; [47:16] gdt base


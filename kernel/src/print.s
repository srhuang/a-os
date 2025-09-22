;-------------------------
; Video GDT selector
;-------------------------
TI_GDT          equ  0
RPL0            equ   0
SELECTOR_VIDEO  equ (0x0003<<3) + TI_GDT + RPL0

section .data
put_int_buffer  dq 0    ; 8 bytes for buffer

[bits 32]
section .text

;-------------------------
; put_char
;   $1 : character to be printed
;-------------------------
global put_char
put_char:
    pushad                  ; backup 8 registers

; set video selector
    mov ax, SELECTOR_VIDEO
    mov gs, ax

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

; get the character
    mov ecx, [esp + 36]     ; pushad 8 registers and return address

; parsing the ascii code
    cmp cl, 0x8             ; Backspace
    jz .backspace
    cmp cl, 0xd             ; Carriage Return (CR)[\r]
    jz .cr
    cmp cl, 0xa
    jz .lf                  ; Line Feed (LF)[\n]

; print the ascii
    shl bx, 1               ; 1 char = 2 bytes
    mov byte [gs:bx], cl
    mov byte [gs:bx+1], 0x07
    shr bx, 1               ; 2 bytes = 1 char
    inc bx

; check roll screen
    cmp bx, 2000            ; 80(column) * 25(row) = 2000
    jl .set_cursor
    jmp .roll_screen

.backspace:
    dec bx
    shl bx, 1               ; 1 char = 2 bytes
    mov byte [gs:bx], 0x20  ; space = 0x20
    mov byte [gs:bx+1], 0x07
    shr bx,1                ; 2 bytes = 1 char
    jmp .set_cursor

.cr:
.lf:
; set bx to next line
; dividend: dx:ax, quotient: ax, remainder: dx
    xor dx, dx              ; clear to zero
    mov ax, bx
    mov si, 80
    div si
    sub bx, dx              ; minus remainder
    add bx, 80              ; next line

; check roll screen
    cmp bx, 2000
    jl .set_cursor

.roll_screen:               ; bx unit is bytes
    mov ecx, 3840           ; 2000-80=1920 chars, 1920*2=3840 bytes
    mov esi, 0xc00b80a0     ; row 1
    mov edi, 0xc00b8000     ; row 0
    cld                     ; set increment
    rep movsb

    mov bx, 3840            ; the last line
    mov ecx, 80             ; clear each column
.cls_loop:
    mov byte [gs:bx], 0x20  ; space
    mov byte [gs:bx+1], 0x07
    add bx, 2
    loop .cls_loop

; set the cursor position (bx unit is char)
    mov bx,1920             ; the head of last line

.set_cursor:
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

    popad                   ; restore 8 registers
    ret

;-------------------------
; put_str
;   $1 : character to be printed
;-------------------------
global put_str
put_str:
    pushad                  ; backup 8 registers

    mov ebx, [esp + 36]     ; pushad 8 registers and return address
    xor ecx, ecx            ; clear to zero

.each_char:
    mov cl, [ebx]
    cmp cl, 0
    jz .str_end
    push ecx                ; push argument
    call put_char
    add esp, 4              ; pop argument
    inc ebx
    jmp .each_char

.str_end:
    popad                   ; restore 8 registers
    ret

;-------------------------
; put_int
;   $1 : integer to be printed
;-------------------------
global put_int
put_int:
    pushad                  ; backup 8 registers

    mov eax, [esp + 36]     ; pushad 8 registers and return address
    mov ebx, put_int_buffer
    mov ecx, 8              ; 32 bits interger, each loop for 4-bit
    mov edx, eax            ; the char to be printed
    mov edi, 7              ; Big endian

.each_4_bit_loop:
    and edx, 0xF
    cmp edx, 9
    jg .is_AtoF
    add edx, '0'
    jmp .store

.is_AtoF:
    sub edx, 10
    add edx, 'A'

.store:
    mov [ebx + edi], dl
    dec edi
    shr eax, 4
    mov edx, eax
    loop .each_4_bit_loop

; ready to print
    mov edi, -1
.skip_zero:
    inc edi
    cmp edi, 8
    je .print_each_char
    mov dl, [ebx + edi]
    cmp dl, '0'
    je .skip_zero

.print_each_char:
    push edx                ; push argument
    call put_char
    add esp, 4              ; pop argument
    inc edi
    mov dl, [ebx, + edi]
    cmp edi, 8
    jl .print_each_char

    popad                   ; restore 8 registers
    ret

;-------------------------
; set_cursor
;   $1 : cursor position
;-------------------------
global set_cursor
set_cursor:
    pushad                  ; backup 8 registers

    mov bx, [esp + 36]      ; pushad 8 registers and return address

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

    popad                   ; restore 8 registers
    ret

;-------------------------
; cls_screen
;   no argument
;-------------------------
global cls_screen
cls_screen:
    pushad                  ; backup 8 registers

; set video selector
    mov ax, SELECTOR_VIDEO
    mov gs, ax

; clear screen
    mov bx, 0
    mov ecx, 2000           ; 80 * 25 = 2000 char
.cls_loop:
    mov byte [gs:bx], 0x20  ; space = 0x20
    mov byte [gs:bx+1], 0x07
    add bx, 2
    loop .cls_loop

; set cursor
    mov bx, 0
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

    popad                   ; restore 8 registers
    ret

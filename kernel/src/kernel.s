[bits 32]
;-------------------------
; external
;-------------------------
extern intr_func
extern syscall_func

;-------------------------
; define
;-------------------------
%define ERROR_CODE  nop         ; CPU push error code
%define ZERO        push 0

;-------------------------
; macro
;-------------------------
%macro VECTOR 2
;   %1  : interrupt number
;   %2  : error code
section .text
intr_%1_entry:
; backup registers
    %2                          ; error code
    push ds
    push es
    push fs
    push gs
    pushad                      ; backup 8 registers

    ; 8259A EOI
    mov al,0x20                 ; EOI
    out 0xA0,al                 ; EOI to slave
    out 0x20,al                 ; EOI to master

    push %1                     ; interrupt number
    call [intr_func + %1 * 4]   ; call the external function
    add esp, 4                  ; clear argument in stack

; restore registers
    popad                       ; restore 8 registers
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4                  ; clear error code in stack
    iretd

section .data
    dd intr_%1_entry
%endmacro

;-------------------------
; interrupt entry
;-------------------------
section .data
global intr_entry
intr_entry:
; internal interrupt
    VECTOR 0x00,ZERO
    VECTOR 0x01,ZERO
    VECTOR 0x02,ZERO
    VECTOR 0x03,ZERO
    VECTOR 0x04,ZERO
    VECTOR 0x05,ZERO
    VECTOR 0x06,ZERO
    VECTOR 0x07,ZERO
    VECTOR 0x08,ERROR_CODE
    VECTOR 0x09,ZERO
    VECTOR 0x0A,ERROR_CODE
    VECTOR 0x0B,ERROR_CODE
    VECTOR 0x0C,ERROR_CODE
    VECTOR 0x0D,ERROR_CODE
    VECTOR 0x0E,ERROR_CODE
    VECTOR 0x0F,ZERO
    VECTOR 0x10,ZERO
    VECTOR 0x11,ERROR_CODE
    VECTOR 0x12,ZERO
    VECTOR 0x13,ZERO
    VECTOR 0x14,ZERO
    VECTOR 0x15,ERROR_CODE
    VECTOR 0x16,ZERO
    VECTOR 0x17,ZERO
    VECTOR 0x18,ZERO
    VECTOR 0x19,ZERO
    VECTOR 0x1A,ZERO
    VECTOR 0x1B,ZERO
    VECTOR 0x1C,ZERO
    VECTOR 0x1D,ERROR_CODE
    VECTOR 0x1E,ERROR_CODE
    VECTOR 0x1F,ZERO
; external interrupt
    VECTOR 0x20,ZERO    ; system timer
    VECTOR 0x21,ZERO    ; keyboard
    VECTOR 0x22,ZERO    ; slave PIC
    VECTOR 0x23,ZERO    ; COM2
    VECTOR 0x24,ZERO    ; COM1
    VECTOR 0x25,ZERO    ; LPT2
    VECTOR 0x26,ZERO    ; floppy
    VECTOR 0x27,ZERO    ; LPT1
    VECTOR 0x28,ZERO    ; RTC
    VECTOR 0x29,ZERO    ; Redirected IRQ2
    VECTOR 0x2A,ZERO    ; General purpose
    VECTOR 0x2B,ZERO    ; General purpose
    VECTOR 0x2C,ZERO    ; mouse
    VECTOR 0x2D,ZERO    ; FPU
    VECTOR 0x2E,ZERO    ; Primary IDE channel
    VECTOR 0x2F,ZERO    ; Secondary IDE channel

;-------------------------
; system call entry
;-------------------------
section .text
global syscall_entry
syscall_entry:
; backup registers
    ZERO                        ; reserved for error code
    push ds
    push es
    push fs
    push gs
    pushad                      ; backup 8 registers

    push edx                    ; argument 3
    push ecx                    ; argument 2
    push ebx                    ; argument 1
    call [syscall_func + eax * 4]   ; call the external function
    add esp, 12                 ; clear argument in stack

; return value stored in EAX
    mov [esp + 7 * 4], eax

; restore registers
    popad                       ; restore 8 registers
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4                  ; clear error code in stack
    iretd

[bits 32]
;-------------------------
; external
;-------------------------
extern main
extern exit

;-------------------------
; _start
;   no argument
;-------------------------
section .text
global _start
_start:
    push ebx    ; push argv
    push ecx    ; push argc
    call main

    push  eax   ; return value
    call exit


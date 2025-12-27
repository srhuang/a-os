[bits 32]
;-------------------------
; external
;-------------------------
extern main

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


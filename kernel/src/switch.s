[bits 32]
;-------------------------
; external variable
;-------------------------

;-------------------------
; define
;-------------------------

;-------------------------
; macro
;-------------------------

;-------------------------
; external function
;-------------------------
section .text
global switch_to
switch_to:
; backup registers based on calling convention
    push esi
    push edi
    push ebx
    push ebp

    mov eax, [esp + 20] ; get argument 1: current task
    mov [eax], esp      ; backup current task esp
    mov eax, [esp + 24] ; get argument 2: next task
    mov esp, [eax]      ; restore next task esp

; restore registers
    pop ebp
    pop ebx
    pop edi
    pop esi

    ret


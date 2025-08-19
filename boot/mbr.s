section mbr vstart=0x7c00

;-------------------------
; Initialize segment registers(sreg)
;-------------------------
    mov ax, 0x0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

;-------------------------
; set MBR stack pointer
;-------------------------
    mov sp, 0x8000

;-------------------------
; get current cursor
;-------------------------
    mov ah, 0x3     ; Function code
    mov bh, 0x0     ; Page Number
    int 0x10        ; BIOS interrupt call
                    ; output: dh=row, dl=column

;-------------------------
; print string
;-------------------------
    mov bp, message ; absolute address of string
    mov ah, 0x13    ; Function code
    mov al, 0x01    ; Write mode
    mov bh, 0x0     ; Page Number
    mov bl, 0x2     ; font color    
    mov cx, 0x5     ; length of string    
                    ; input: dh=row, dl=column
    int 0x10        ; BIOS interrupt call


;-------------------------
; pause the process
;-------------------------
    jmp $           ; stop here!!!

;-------------------------
; data
;-------------------------
    message db "1 MBR"
    times 510-($-$$) db 0
    db 0x55,0xaa

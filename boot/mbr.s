%include "boot.inc"
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
; set stack pointer
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
; load loader
;-------------------------
    mov eax, LOADER_START_SECTOR
    mov bx, LOADER_BASE_ADDR
    mov cl, LOADER_SECTOR_COUNT
    call read_hd

;-------------------------
; jump to loader
;-------------------------
    jmp LOADER_START_ADDR

;-------------------------
; read hard disk
;   eax : LBA
;   ebx : RAM address
;   cl  : sector count
;-------------------------
read_hd:

; step 1 : set sector count
    mov esi, eax    ; backup eax
                    ; cause port operation only accept al/ax
    mov dx, 0x1f2
    mov al, cl
    out dx, al
    mov eax, esi    ; restore eax

; step 2 : set LBA
    ; LBA[7:0]
    mov dx, 0x1f3
    out dx, al

    ; LBA[15:8]
    shr eax, 8
    mov dx, 0x1f4
    out dx, al

    ; LBA[23:16]
    shr eax, 8
    mov dx, 0x1f5
    out dx, al

    ; LBA[27:24] and setting
    shr eax, 8
    and al, 0x0f
    or al, 0xe0     ; b'1110 for master drive / LBA addressing
    mov dx, 0x1f6
    out dx, al

; step 3 : start read command
    mov dx, 0x1f7
    mov al,0x20
    out dx, al

; step 4 : polling status
    polling:
        nop             ; hardware delay
        in al, dx
        and al, 0x88    ; [4]:DRQ, [7]:BSY
        cmp al, 0x08    ; Data is ready and not busy
        jnz polling     ; keep polling

; step 5 : read data from hard disk
    mov al, cl
    mov dx, 256     ; sector count * 512 / 2 
                    ; (2 byte for each read)
    mul dx          ; ax = ax * dx (only support ax)
    mov cx, ax      ; cx = total loop time
    mov dx, 0x1f0
    read:
        in ax, dx
        mov [bx], ax
        add bx, 2
        loop read
    ret

;-------------------------
; data
;-------------------------
    message db "1 MBR"
    times 510-($-$$) db 0
    db 0x55,0xaa

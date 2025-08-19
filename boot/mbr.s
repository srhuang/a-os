section mbr
    jmp $ ; 程式會停在這裡
    times 510-($-$$) db 0
    db 0x55,0xaa

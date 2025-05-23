bits 16 ; We are in 16-bit real mode

org 0x7c00 ; BIOS loads our bootloader at this address

start:
    mov si, message ; Point SI to our message
print_char:
    lodsb           ; Load byte from [SI] into AL, and increment SI
    or al, al       ; Check if AL is zero (end of string)
    jz halt         ; If zero, jump to halt
    mov ah, 0x0e    ; BIOS teletype output function
    mov bh, 0       ; Page number
    mov bl, 0x07    ; White text on black background
    int 0x10        ; Call BIOS video interrupt
    jmp print_char  ; Loop to print next character

halt:
    cli             ; Clear interrupts
    hlt             ; Halt the processor

message:
    db "Hello", 0   ; Null-terminated string

; Padding and magic number
times 510 - ($-$$) db 0 ; Pad remainder of boot sector with 0s
dw 0xaa55             ; Boot signature

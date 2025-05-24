bits 16 ; We are in 16-bit real mode

org 0x7c00 ; BIOS loads our bootloader at this address

start:
    ; Switch to VGA graphics mode 13h (320x200, 256 colors)
    mov ah, 0x00    ; Set video mode function
    mov al, 0x13    ; Mode 13h
    int 0x10        ; Call BIOS video interrupt

    ; Fill screen with blue (color code 1)
    mov ax, 0xA000
    mov es, ax          ; ES points to video memory segment A000h
    xor di, di          ; DI = 0 (start offset in video memory)
    mov cx, 320*200     ; Number of pixels (320*200 = 64000 bytes)
    mov al, 0x01        ; Color blue (palette index 1)
    rep stosb           ; Fill CX bytes at ES:[DI] with AL, inc DI, dec CX

    ; After filling screen, proceed to enable A20 line and other setup
    jmp enable_a20

halt:
    cli             ; Clear interrupts
    hlt             ; Halt the processor

; Enable A20 Line using Keyboard Controller
enable_a20:
    call    a20_wait_input_empty    ; Wait for input buffer to be empty
    mov     al, 0xD1                ; Command to write to output port P2
    out     0x64, al                ; Send command to status port

    call    a20_wait_input_empty    ; Wait for input buffer to be empty
    mov     al, 0xDF                ; Data to send: Enable A20 (bit 1), other bits set as typical
                                    ; (bit 0=system reset, bit 1=A20, ... we want 11011111)
    out     0x60, al                ; Send data to data port

    call    a20_wait_input_empty    ; Wait for input buffer to be empty
    ; A20 line should now be enabled.
    jmp     load_gdt                ; Jump to load GDT (or next step)

a20_wait_input_empty:
    in      al, 0x64                ; Read status port
    test    al, 0x02                ; Test bit 1 (input buffer full)
    jnz     a20_wait_input_empty    ; If not zero (buffer not empty), loop
    ret

load_gdt:
    lgdt [gdt_descriptor] ; Load GDT register

    cli                     ; Disable interrupts
    mov eax, cr0            ; Load CR0
    or eax, 0x1             ; Set PE bit (bit 0)
    mov cr0, eax            ; Write back to CR0 - now in protected mode!
    
    jmp 0x08:protected_mode_entry ; Far jump to 32-bit code segment (selector 0x08)


; Global Descriptor Table (GDT)
gdt_start:
    ; Null Descriptor (8 bytes)
    dq 0x0000000000000000

    ; Code Segment Descriptor (selector 0x08)
    ; Base=0x00000000, Limit=0xFFFFF (4GB with G=1)
    ; Access Byte: 0x9A (Present, DPL 0, Code, Executable, Readable)
    ; Flags + Upper Limit: 0xCF (Granularity=1, 32-bit, Limit 16-19)
    dw 0xFFFF    ; Limit (low bits 0-15)
    dw 0x0000    ; Base (low bits 0-15)
    db 0x00      ; Base (mid bits 16-23)
    db 0x9A      ; Access Byte (P=1, DPL=0, S=1, Type=Code, E=1, R=1, A=0)
    db 0xCF      ; Granularity (G=1, D=1, L=0, AVL=0) + Limit (high bits 16-19)
    db 0x00      ; Base (high bits 24-31)

    ; Data Segment Descriptor (selector 0x10)
    ; Base=0x00000000, Limit=0xFFFFF (4GB with G=1)
    ; Access Byte: 0x92 (Present, DPL 0, Data, Writable)
    ; Flags + Upper Limit: 0xCF (Granularity=1, 32-bit, Limit 16-19)
    dw 0xFFFF    ; Limit (low bits 0-15)
    dw 0x0000    ; Base (low bits 0-15)
    db 0x00      ; Base (mid bits 16-23)
    db 0x92      ; Access Byte (P=1, DPL=0, S=1, Type=Data, W=1, A=0)
    db 0xCF      ; Granularity (G=1, D=1, L=0, AVL=0) + Limit (high bits 16-19)
    db 0x00      ; Base (high bits 24-31)
gdt_end:

; GDT Descriptor (GDTR)
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; GDT Limit (size of GDT - 1)
    dd gdt_start               ; GDT Base Address (linear address of gdt_start)


protected_mode_entry:
    bits 32         ; We are now in 32-bit Protected Mode

    ; Set up data segments
    mov ax, 0x10    ; Our data segment selector (0x10, third entry in GDT)
    mov ds, ax
    mov es, ax
    ; fs and gs are not strictly necessary for this simple example but good practice
    mov fs, ax 
    mov gs, ax
    mov ss, ax

    ; Kernel copy routine
    ; Source: 0x7E00 (where kernel is loaded after bootloader)
    ; Destination: 0x100000 (where kernel will run)
    ; Size: 8KB = 8192 bytes = 2048 dwords (double words)

    mov esi, 0x7E00         ; Source address
    mov edi, 0x100000       ; Destination address
    mov ecx, 2048           ; Number of dwords to copy (8192 bytes / 4 bytes/dword)
    
    cld                     ; Clear direction flag (so LODSD/STOSD increment ESI/EDI)

copy_loop:
    lodsd                   ; Load dword from [ds:esi] into eax, increment esi
    stosd                   ; Store dword from eax into [es:edi], increment edi
    loop copy_loop          ; Decrement ecx, loop if ecx != 0

    ; Jump to the kernel's entry point at 0x100000
    jmp 0x100000

; Padding and magic number
times 510 - ($-$$) db 0 ; Pad remainder of boot sector with 0s
dw 0xaa55             ; Boot signature
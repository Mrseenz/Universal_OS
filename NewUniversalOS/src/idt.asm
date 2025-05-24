bits 32

%define KERNEL_CODE_SEGMENT 0x08 ; From GDT

; Macro to create an IDT entry
; Args: address, selector, flags
%macro IDT_ENTRY 3
    dw %1 & 0xFFFF          ; Offset low
    dw %2                   ; Selector
    db 0                    ; Reserved (IST)
    db %3                   ; Type/Attributes (P, DPL, S, GateType)
    dw (%1 >> 16) & 0xFFFF  ; Offset high
%endmacro

global idt_load
extern isr_handler_c ; C function to handle interrupts

idt_load:
    mov eax, [esp+4] ; Get the idt_ptr argument from stack
    lidt [eax]       ; Load IDT register
    ret

; ISR Macros (modified for clarity and to pass registers_t* to C handler)
%macro ISR_NO_ERRCODE 1
global isr%1
isr%1:
    cli
    push byte 0     ; Dummy error code
    push byte %1    ; Interrupt number
    jmp common_isr_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    ; Error code is already on stack
    push byte %1    ; Interrupt number (comes after error code on stack)
    jmp common_isr_stub
%endmacro

; Macro for IRQ Handlers (pushes dummy error code for stack consistency)
; Args: ISR_NUM (e.g., 32 for IRQ0), INT_NUM (actual interrupt vector number)
%macro IRQ_HANDLER_STUB 2
global isr%1
isr%1:
    cli
    push byte 0     ; Push a dummy error code
    push byte %2    ; Push the interrupt number
    jmp common_isr_stub
%endmacro

common_isr_stub:
    pushad          ; Pushes eax, ecx, edx, ebx, esp, ebp, esi, edi (esp is original value)
    
    mov ax, ds      ; Save original data segment
    push eax
    
    mov ax, 0x10    ; Load kernel data segment (selector 0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp        ; Pass pointer to the stack (which now looks like registers_t) to isr_handler_c
    call isr_handler_c
    add esp, 4      ; Clean up stack pointer argument
    
    pop ebx         ; Restore original data segment (saved in eax, but pushad saved ds in order, so pop to ebx temporarily)
    mov ds, bx      ; (Corrected: pop into a register then mov to ds)
    mov es, bx      ; Also restore es, fs, gs if they were changed, or ensure they are valid.
    mov fs, bx      ; For simplicity, assume they all take kernel data segment for now after interrupt.
    mov gs, bx      ; This part needs care; kernel should operate in its own segments.
                    ; The C handler will run with kernel DS. Upon iret, original segments are restored if CPL changes.

    popad           ; Pop all general registers
    
    ; Clean up interrupt number and error code from stack
    ; Error code (if present) is below int_num. int_num is at [esp+4], err_code at [esp] for isr_handler_c
    ; After popad, esp points to the pushed interrupt number.
    ; Then error code is below it.
    add esp, 8      ; Pop interrupt number and error code
    
    sti             ; Re-enable interrupts (IF THIS IS DESIRED FOR ALL ISRs - often not for exceptions)
                    ; For exceptions, iret should restore previous IF state.
                    ; For IRQs, this is usually done by the handler or just before iret.
                    ; Let's make it default for now, can be refined.
    iret            ; Return from interrupt

; ISR Definitions (Exceptions 0-31)
ISR_NO_ERRCODE  0   ; Divide by zero
ISR_NO_ERRCODE  1   ; Debug
ISR_NO_ERRCODE  2   ; Non-maskable interrupt
ISR_NO_ERRCODE  3   ; Breakpoint
ISR_NO_ERRCODE  4   ; Overflow
ISR_NO_ERRCODE  5   ; Bound range exceeded
ISR_NO_ERRCODE  6   ; Invalid opcode
ISR_NO_ERRCODE  7   ; Device not available
ISR_ERRCODE     8   ; Double fault
ISR_NO_ERRCODE  9   ; Coprocessor segment overrun
ISR_ERRCODE     10  ; Invalid TSS
ISR_ERRCODE     11  ; Segment not present
ISR_ERRCODE     12  ; Stack-segment fault
ISR_ERRCODE     13  ; General protection fault
ISR_ERRCODE     14  ; Page fault
ISR_NO_ERRCODE  15  ; Reserved
ISR_NO_ERRCODE  16  ; x87 FPU floating-point error
ISR_ERRCODE     17  ; Alignment check
ISR_NO_ERRCODE  18  ; Machine check
ISR_NO_ERRCODE  19  ; SIMD floating-point exception
ISR_NO_ERRCODE  20  ; Virtualization exception
ISR_NO_ERRCODE  21  ; Control protection exception
; 22-27 Reserved
ISR_NO_ERRCODE  22
ISR_NO_ERRCODE  23
ISR_NO_ERRCODE  24
ISR_NO_ERRCODE  25
ISR_NO_ERRCODE  26
ISR_NO_ERRCODE  27
ISR_NO_ERRCODE  28  ; Hypervisor injection exception
ISR_NO_ERRCODE  29  ; VMM communication exception
ISR_ERRCODE     30  ; Security exception
ISR_NO_ERRCODE  31  ; Reserved

; IRQ Handler Stubs (INT 32-47)
IRQ_HANDLER_STUB 32, 32  ; ISR stub for INT 32 (IRQ 0 - Timer)
IRQ_HANDLER_STUB 33, 33  ; ISR stub for INT 33 (IRQ 1 - Keyboard)
; Add more for IRQ2 through IRQ15 later if needed:
; ...
; IRQ_HANDLER_STUB 47, 47  ; ISR stub for INT 47 (IRQ 15)


; PIC Remapping
; Master PIC: 0x20 (CMD), 0x21 (DATA)
; Slave PIC:  0xA0 (CMD), 0xA1 (DATA)

%define PIC_MASTER_CMD  0x20
%define PIC_MASTER_DATA 0x21
%define PIC_SLAVE_CMD   0xA0
%define PIC_SLAVE_DATA  0xA1

%define PIC_EOI         0x20 ; End-Of-Interrupt command

global pic_remap
global pic_send_eoi ; For sending End of Interrupt signal

pic_remap:
    pushad ; Save all registers

    ; ICW1: Start initialization sequence in cascade mode
    mov al, 0x11
    out PIC_MASTER_CMD, al
    jmp $ + 2 ; Short delay
    out PIC_SLAVE_CMD, al
    jmp $ + 2

    ; ICW2: New master PIC vector offset (0x20 = 32)
    mov al, 0x20
    out PIC_MASTER_DATA, al
    jmp $ + 2

    ; ICW2: New slave PIC vector offset (0x28 = 40)
    mov al, 0x28
    out PIC_SLAVE_DATA, al
    jmp $ + 2

    ; ICW3: Tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    mov al, 0x04
    out PIC_MASTER_DATA, al
    jmp $ + 2

    ; ICW3: Tell Slave PIC its cascade identity (0000 0010)
    mov al, 0x02
    out PIC_SLAVE_DATA, al
    jmp $ + 2

    ; ICW4: Set 8086/88 (MCS-80) mode
    mov al, 0x01
    out PIC_MASTER_DATA, al
    jmp $ + 2
    out PIC_SLAVE_DATA, al
    jmp $ + 2

    ; Mask all interrupts on both PICs initially (write 0xFF to data ports)
    ; Interrupts will be unmasked one by one as handlers are written.
    mov al, 0xFF
    out PIC_MASTER_DATA, al
    out PIC_SLAVE_DATA, al

    popad ; Restore all registers
    ret

; Send End-Of-Interrupt signal to PIC(s)
; Argument: irq_num (0-15)
pic_send_eoi:
    pushad
    movzx eax, byte [esp + 32 + 4] ; Get irq_num (after pushad + ret addr)

    cmp al, 8
    jl master_eoi ; If IRQ < 8, it's on master PIC

slave_eoi:
    mov al, PIC_EOI
    out PIC_SLAVE_CMD, al ; Send EOI to slave PIC
    ; Fall through to send EOI to master PIC as well for cascaded IRQs

master_eoi:
    mov al, PIC_EOI
    out PIC_MASTER_CMD, al ; Send EOI to master PIC
    
    popad
    ret

; IDT Table (256 entries)
idt_data:
    %assign i 0
    %rep 256
        ; Default: points to a generic handler or is marked not present
        ; For now, let's point defined ISRs and leave others as 0 (non-present/invalid)
        ; or point them to a default handler that just halts.
        ; We'll set these up properly from C code.
        ; This is just space reservation.
        IDT_ENTRY 0, 0, 0 ; Placeholder, will be filled by C code
    %endrep
idt_end:

idt_ptr_struct:
    dw idt_end - idt_data - 1 ; IDT Limit
    dd idt_data               ; IDT Base Address
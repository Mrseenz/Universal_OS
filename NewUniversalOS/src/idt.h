#ifndef IDT_H
#define IDT_H

#include <stdint.h> // For uintN_t types

// Structure to hold register values passed from ISR stubs
typedef struct {
    uint32_t ds;                                            // Data segment selector
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;   // Pushed by pusha. esp_dummy is stack pointer before pusha.
    uint32_t int_no, err_code;                              // Interrupt number and error code (if any)
    uint32_t eip, cs, eflags, useresp, ss;                  // Pushed by the processor automatically
} registers_t;

// Structure for an IDT entry (gate)
typedef struct {
    uint16_t base_low;    // Lower 16 bits of handler function address
    uint16_t selector;    // Kernel segment selector
    uint8_t  always0;     // This must always be zero
    uint8_t  flags;       // Type and attributes (P, DPL, S, GateType)
    uint16_t base_high;   // Upper 16 bits of handler function address
} __attribute__((packed)) idt_entry_t; // Ensure packed structure

// Structure for the IDT pointer (to be loaded with lidt)
typedef struct {
    uint16_t limit;       // Size of IDT - 1
    uint32_t base;        // Base address of the IDT
} __attribute__((packed)) idt_ptr_t;

// This function will be implemented in C (interrupts.c) and called from common_isr_stub
extern void isr_handler_c(registers_t* regs); // Changed to pointer as per previous correction

// This function will be implemented in idt.asm and called from C to load the IDT
extern void idt_load(void* idt_ptr); // Argument is idt_ptr_t*

// ISR stubs (implemented in idt.asm)
extern void isr0(); extern void isr1(); extern void isr2(); extern void isr3();
extern void isr4(); extern void isr5(); extern void isr6(); extern void isr7();
extern void isr8(); extern void isr9(); extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();
extern void isr32(); // For IRQ0 (Timer)
extern void isr33(); // For IRQ1 (Keyboard)
// extern void isr34(); ... // For future IRQs

// PIC Remapping function (implemented in idt.asm)
extern void pic_remap(void);

// Send End Of Interrupt signal (implemented in idt.asm)
// Takes IRQ number (0-15)
extern void pic_send_eoi(unsigned char irq_num);

// Function to initialize IDT (will be in kernel.c)
void idt_init(void); 

#endif // IDT_H

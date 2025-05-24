#include <stdint.h>    // For uintN_t types
#include "vga_text.h"
#include "idt.h"
#include "pit.h"
#include "serial.h"
#include "ports.h"    // For inb/outb for PIC unmasking

// Global IDT and IDT pointer (definitions, not extern)
idt_entry_t idt[256];
idt_ptr_t idt_p;

// Function to set an IDT entry (defined in kernel.c)
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

// Initialize the IDT (defined in kernel.c)
void idt_init() {
    idt_p.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_p.base = (uint32_t)&idt;

    unsigned char *idt_ptr_byte = (unsigned char *)&idt;
    for(unsigned int i = 0; i < sizeof(idt_entry_t) * 256; i++) {
        idt_ptr_byte[i] = 0;
    }

    // ISRs 0-31 for exceptions
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);
    
    // Setup IRQ0 (Timer) mapped to INT 32
    idt_set_gate(32, (uint32_t)isr32, 0x08, 0x8E);
    // Setup IRQ1 (Keyboard) mapped to INT 33
    idt_set_gate(33, (uint32_t)isr33, 0x08, 0x8E); 

    pic_remap();      // Remap the PIC
    idt_load(&idt_p); // Load the IDT pointer
}

// Kernel entry point
void kmain() {
    // Initialize VGA and clear screen
    vga_clear_screen(0x07); // White on black

    // Initialize COM1 serial port
    serial_init();
    serial_print_string("Serial COM1 Initialized.\n");

    // Initialize Interrupt Descriptor Table and Programmable Interrupt Controllers
    idt_init(); 
    serial_print_string("IDT and PICs configured.\n");
    vga_print_string("IDT and PICs configured.\n", 0x0A); // Green on Black for VGA

    // Print 'K' to VGA and Serial
    vga_set_cursor_pos(0,0);
    vga_print_char('K', 0x2F); // Green background, White foreground
    serial_print_string("K - Kernel booted.\n");

    // Initialize Programmable Interval Timer (PIT)
    pit_init(100); // Configure PIT to ~100Hz and unmask IRQ0
    serial_print_string("PIT Initialized (100Hz), IRQ0 Unmasked.\n");
    vga_print_string("PIT Initialized (100Hz), IRQ0 Unmasked.\n", 0x0A); // Green on Black for VGA
    
    // Unmask IRQ1 (Keyboard)
    serial_print_string("Unmasking IRQ1 (Keyboard)...\n");
    vga_print_string("Unmasking IRQ1 (Keyboard)...\n", 0x0A);
    uint8_t pic_mask = inb(0x21); // Read current mask from Master PIC data port (0x21)
    outb(0x21, pic_mask & 0xFD);   // Clear bit 1 (0xFD = 11111101b) to unmask IRQ1
       
    // Enable interrupts
    asm volatile ("sti");
    serial_print_string("Interrupts Enabled.\n");

    // The divide-by-zero test for Exception 0 should be commented out
    // to allow the timer interrupt (IRQ0 / INT 32) to be the primary interrupt tested.
    /*
    vga_print_string("Attempting divide by zero...\n", 0x0F);
    serial_print_string("Attempting divide by zero...\n");
    int x = 5;
    int y = 0;
    int z = x / y; 
    if (z == 123) { // To use z, won't be reached
        vga_print_string("This should not print!\n", 0x0C);
        serial_print_string("This should not print!\n");
    }
    */

    // Idle loop
    while (1) {
        asm volatile ("hlt");
    }
}
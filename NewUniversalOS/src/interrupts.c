#include "idt.h"
#include "vga_text.h"
#include "serial.h"   // For serial_print_* functions
#include "ports.h"    // For inb function
#include <stdint.h>   // For uintN_t types

// Array of exception messages
const char *exception_messages[] = {
    "Divide By Zero Error", "Debug", "Non Maskable Interrupt", "Breakpoint", "Overflow",
    "Bound Range Exceeded", "Invalid Opcode", "Device Not Available", "Double Fault",
    "Coprocessor Segment Overrun", "Invalid TSS", "Segment Not Present", "Stack Segment Fault",
    "General Protection Fault", "Page Fault", "Reserved", "x87 Floating Point", "Alignment Check",
    "Machine Check", "SIMD Floating Point", "Virtualization Exception", "Control Protection Exception",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Hypervisor Injection Exception", "VMM Communication Exception", "Security Exception", "Reserved"
};

volatile uint32_t timer_ticks = 0;
char spinner_chars[] = {'-', '\\', '|', '/'}; // Note: double backslash for literal backslash
uint8_t spinner_idx = 0;
const int VGA_WIDTH_CONST_INT = 80; 

void isr_handler_c(registers_t* regs) {
    char default_attr = 0x0F; // White on Black for general messages
    char irq_attr = 0x0A;     // Green on Black for IRQ messages

    if (regs->int_no == 32) { // Timer Interrupt (IRQ0)
        timer_ticks++;

        // Serial print for timer ticks
        if ((timer_ticks % 100) == 0) { // Approx every second if PIT is 100Hz
            serial_print_string("Timer tick: ");
            serial_print_dec(timer_ticks);
            serial_print_string(" (Serial)\n");
        }

        // VGA spinner logic
        if ((timer_ticks % 20) == 0) { 
            int old_row, old_col;
            vga_get_cursor_pos(&old_row, &old_col);
            vga_set_cursor_pos(0, VGA_WIDTH_CONST_INT - 1); 
            vga_print_char(spinner_chars[spinner_idx], 0x0E); // Yellow on Black
            vga_set_cursor_pos(old_row, old_col);     
            spinner_idx = (spinner_idx + 1) % 4;
        }
        
        pic_send_eoi(0); // Send EOI for IRQ0
        return;          // Return from interrupt for IRQs
    } else if (regs->int_no == 33) { // Keyboard Interrupt (IRQ1)
        uint8_t scancode = inb(0x60); // Read scancode from keyboard controller data port

        // Print to serial
        serial_print_string("Keyboard Scancode (IRQ1): ");
        serial_print_hex((uint32_t)scancode); // Cast to uint32_t for serial_print_hex
        serial_print_string("\n");

        // Also print to VGA (optional, but good for consistency if VGA was working)
        // Example:
        // vga_set_cursor_pos(6, 0); // Or some other designated line for keyboard
        // vga_print_string("Keyboard Scancode: ", 0x0B); // Light Cyan on Black
        // vga_print_hex(scancode, 0x0B);
        // vga_print_string("\n", 0x0B);

        pic_send_eoi(1); // Send EOI for IRQ1 (keyboard is on master PIC, IRQ number 1)
        return; 
    }
    
    // For non-timer/non-keyboard interrupts, print messages starting from row 5 to VGA
    // and also to serial.
    vga_set_cursor_pos(5, 0); 
    
    vga_print_string("Received Interrupt: ", default_attr);
    vga_print_dec(regs->int_no, default_attr);
    serial_print_string("Received Interrupt: "); 
    serial_print_dec(regs->int_no);
    
    if (regs->int_no < 32) { // CPU Exception
        vga_print_string(" (", default_attr);
        serial_print_string(" (");
        if (regs->int_no < sizeof(exception_messages)/sizeof(const char*)) {
            vga_print_string(exception_messages[regs->int_no], default_attr);
            serial_print_string(exception_messages[regs->int_no]);
        } else {
            vga_print_string("Unknown Exception", default_attr);
            serial_print_string("Unknown Exception");
        }
        vga_print_string(")", default_attr);
        serial_print_string(")");

        vga_print_string("\nError Code: ", default_attr);
        vga_print_hex(regs->err_code, default_attr);
        serial_print_string("\nError Code: ");
        serial_print_hex(regs->err_code);

        vga_print_string("\nSystem Halted!\n", 0x0C); // Red on Black for halt message
        serial_print_string("\nSystem Halted!\n");
        
        asm volatile ("cli; hlt");
    } else if (regs->int_no > 33 && regs->int_no < 48) { // Adjusted condition to exclude 33
        vga_print_string(" (IRQ ", irq_attr);
        vga_print_dec(regs->int_no - 32, irq_attr); 
        vga_print_string(")\n", irq_attr);

        serial_print_string(" (IRQ ");
        serial_print_dec(regs->int_no - 32);
        serial_print_string(")\n");

        pic_send_eoi((unsigned char)(regs->int_no - 32));
        return; 
    } else { 
        vga_print_string(" (Unknown Interrupt Type)\n", default_attr);
        serial_print_string(" (Unknown Interrupt Type)\n");

        if (regs->int_no >= 32 && regs->int_no < 48) { // If it's an IRQ we didn't specifically handle
             pic_send_eoi((unsigned char)(regs->int_no - 32)); // Still send EOI
             return; 
        } else {
            // If it's an unexpected interrupt number not from PICs and not an exception,
            vga_print_string("Unexpected interrupt vector. System Halted!\n", 0x0C);
            serial_print_string("Unexpected interrupt vector. System Halted!\n");
            asm volatile("cli; hlt");
        }
    }
}
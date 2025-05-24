#include "serial.h"
#include "ports.h" // For inb, outb

// Define COM port registers relative to the base COM1_PORT
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_INT_ENABLE_PORT(base)    (base + 1)
#define SERIAL_FIFO_CTRL_PORT(base)     (base + 2)
#define SERIAL_LINE_CTRL_PORT(base)     (base + 3)
#define SERIAL_MODEM_CTRL_PORT(base)    (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

void serial_init() {
   outb(SERIAL_INT_ENABLE_PORT(COM1_PORT), 0x00);    // Disable all interrupts for COM1
   outb(SERIAL_LINE_CTRL_PORT(COM1_PORT), 0x80);     // Enable DLAB (set baud rate divisor)
   outb(SERIAL_DATA_PORT(COM1_PORT), 0x01);          // Set divisor to 1 (lo byte) for 115200 baud
   outb(SERIAL_INT_ENABLE_PORT(COM1_PORT), 0x00);    //                  (hi byte)
   outb(SERIAL_LINE_CTRL_PORT(COM1_PORT), 0x03);     // 8 bits, no parity, one stop bit (8N1)
   outb(SERIAL_FIFO_CTRL_PORT(COM1_PORT), 0xC7);     // Enable FIFO, clear them, with 14-byte threshold
   outb(SERIAL_MODEM_CTRL_PORT(COM1_PORT), 0x0B);    // IRQs enabled, RTS/DSR set
   // Test loopback mode to see if serial is working (optional)
   // outb(SERIAL_MODEM_CTRL_PORT(COM1_PORT), 0x1E); // Set in loopback mode, test chip
   // outb(SERIAL_DATA_PORT(COM1_PORT), 0xAE);    // Test serial chip (send byte)
   // if(inb(SERIAL_DATA_PORT(COM1_PORT)) != 0xAE) {
   //    return; // Faulty serial chip or setup
   // }
   // outb(SERIAL_MODEM_CTRL_PORT(COM1_PORT), 0x0F); // Restore normal operation
}

int serial_is_transmit_empty() {
   return inb(SERIAL_LINE_STATUS_PORT(COM1_PORT)) & 0x20; // Check bit 5 (Transmitter Holding Register Empty)
}

void serial_write_char(char a) {
   while (serial_is_transmit_empty() == 0); // Wait until TX buffer is empty
   outb(SERIAL_DATA_PORT(COM1_PORT), a);
}

void serial_print_string(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_write_char(str[i]);
    }
}

void serial_print_hex(uint32_t n) {
    serial_write_char('0');
    serial_write_char('x');
    char hex_chars[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; --i) {
        serial_write_char(hex_chars[(n >> (i * 4)) & 0xF]);
    }
}

void serial_print_dec(uint32_t n) {
    if (n == 0) {
        serial_write_char('0');
        return;
    }
    char buffer[12]; 
    int i = 0;
    while (n > 0) {
        buffer[i++] = (n % 10) + '0';
        n /= 10;
    }
    // buffer now holds digits in reverse order
    while (i > 0) {
        serial_write_char(buffer[--i]);
    }
}
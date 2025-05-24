#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h> // For standard integer types

#define COM1_PORT 0x3F8 // Base I/O port for COM1

void serial_init(void);
int serial_is_transmit_empty(void);
void serial_write_char(char a);
void serial_print_string(const char* str);
void serial_print_hex(uint32_t n); // For printing numbers in hex
void serial_print_dec(uint32_t n); // For printing numbers in decimal

#endif // SERIAL_H
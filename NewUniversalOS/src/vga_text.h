#ifndef VGA_TEXT_H
#define VGA_TEXT_H

#include <stddef.h> // For size_t, though not strictly used yet

void vga_set_cursor_pos(int row, int col);
void vga_get_cursor_pos(int* row, int* col);
void vga_print_char(char character, char attribute); // Prints at current cursor, advances cursor
void vga_print_string(const char* str, char attribute);
void vga_print_hex(unsigned int n, char attribute);
void vga_print_dec(unsigned int n, char attribute);
void vga_clear_screen(char attribute);

#endif // VGA_TEXT_H
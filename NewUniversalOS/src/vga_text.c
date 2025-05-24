#include "vga_text.h"

volatile unsigned short* video_memory = (unsigned short*)0xB8000;
int cursor_row = 0;
int cursor_col = 0;
const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;

void vga_set_cursor_pos(int row, int col) {
    cursor_row = row;
    cursor_col = col;
    // Code to update hardware cursor could go here if desired (via outb to 0x3D4/0x3D5)
    // For now, we manage it purely in software for printing.
}

void vga_get_cursor_pos(int* row, int* col) {
    *row = cursor_row;
    *col = cursor_col;
}

void vga_scroll() {
    // Shift all lines up by one
    for (int r = 0; r < VGA_HEIGHT - 1; ++r) {
        for (int c = 0; c < VGA_WIDTH; ++c) {
            video_memory[r * VGA_WIDTH + c] = video_memory[(r + 1) * VGA_WIDTH + c];
        }
    }
    // Clear the last line
    char space_attr = (video_memory[(VGA_HEIGHT-2)*VGA_WIDTH] >> 8); // Use attribute of line above
    if (space_attr == 0) space_attr = 0x07; // Default if screen was blank
    for (int c = 0; c < VGA_WIDTH; ++c) {
        video_memory[(VGA_HEIGHT - 1) * VGA_WIDTH + c] = (space_attr << 8) | ' ';
    }
    cursor_row = VGA_HEIGHT - 1;
    cursor_col = 0;
}

void vga_print_char(char character, char attribute) {
    if (character == '\n') {
        cursor_col = 0;
        cursor_row++;
    } else if (character == '\r') {
        cursor_col = 0;
    } else {
        if (cursor_col >= VGA_WIDTH) {
            cursor_col = 0;
            cursor_row++;
        }
        if (cursor_row >= VGA_HEIGHT) {
            vga_scroll();
        }
        video_memory[cursor_row * VGA_WIDTH + cursor_col] = (attribute << 8) | character;
        cursor_col++;
    }
}

void vga_print_string(const char* str, char attribute) {
    while (*str) {
        vga_print_char(*str++, attribute);
    }
}

void vga_print_hex(unsigned int n, char attribute) {
    vga_print_string("0x", attribute);
    char hex_chars[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; --i) {
        vga_print_char(hex_chars[(n >> (i * 4)) & 0xF], attribute);
    }
}

void vga_print_dec(unsigned int n, char attribute) {
    if (n == 0) {
        vga_print_char('0', attribute);
        return;
    }
    char buffer[12]; // Max 10 digits for 32-bit uint, plus sign and null
    int i = 0;
    while (n > 0) {
        buffer[i++] = (n % 10) + '0';
        n /= 10;
    }
    buffer[i] = '\0';
    // Reverse buffer
    for (int j = 0; j < i / 2; ++j) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
    vga_print_string(buffer, attribute);
}

void vga_clear_screen(char attribute) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        video_memory[i] = (attribute << 8) | ' ';
    }
    vga_set_cursor_pos(0, 0);
}
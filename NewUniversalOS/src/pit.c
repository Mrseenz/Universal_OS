#include "pit.h"
#include "ports.h" // For outb, inb

#define PIT_CMD_PORT    0x43
#define PIT_CHANNEL0_DATA_PORT 0x40
// PIT_CHANNEL1_DATA_PORT 0x41
// PIT_CHANNEL2_DATA_PORT 0x42

#define PIC_MASTER_DATA 0x21 // For unmasking IRQ0

void pit_init(uint32_t frequency) {
    uint32_t divisor = 1193182 / frequency;
    if (divisor == 0) divisor = 1; // Avoid division by zero if frequency is too high
    if (divisor > 65535) divisor = 65535; // Max 16-bit divisor

    // Send command byte: Channel 0, Access LSB then MSB, Mode 2 (Rate Generator)
    // Command: 00110100b = 0x34
    outb(PIT_CMD_PORT, 0x34);

    // Send divisor LSB then MSB
    uint8_t lsb = (uint8_t)(divisor & 0xFF);
    uint8_t msb = (uint8_t)((divisor >> 8) & 0xFF);
    outb(PIT_CHANNEL0_DATA_PORT, lsb);
    outb(PIT_CHANNEL0_DATA_PORT, msb);

    // Unmask IRQ0 (timer) on the master PIC
    // Read current mask, clear bit 0 (IRQ0), write back
    uint8_t current_mask = inb(PIC_MASTER_DATA);
    outb(PIC_MASTER_DATA, current_mask & 0xFE); // 0xFE = 11111110b
}
#include "serial.h"
#include <io.h>

bool serial_init(uint16_t port) {
    outb(port + 1, 0x00);   /* disable interrupts */
    outb(port + 3, 0x80);   /* enable DLAB (baud divisor) */
    outb(port + 0, 0x03);   /* 38400 baud low  */
    outb(port + 1, 0x00);   /* 38400 baud high */
    outb(port + 3, 0x03);   /* 8-N-1 */
    outb(port + 2, 0xC7);   /* FIFO on, clear, 14-byte threshold */
    outb(port + 4, 0x0B);   /* IRQs on, RTS/DSR on */
    outb(port + 4, 0x1E);   /* loopback test */
    outb(port + 0, 0xAE);   /* test byte */
    if (inb(port + 0) != 0xAE) return false;
    outb(port + 4, 0x0F);   /* normal operation */
    return true;
}

static bool serial_transmit_empty(uint16_t port) {
    return (inb(port + 5) & 0x20) != 0;
}

void serial_putchar(uint16_t port, char c) {
    while (!serial_transmit_empty(port));
    outb(port, (uint8_t)c);
}

void serial_puts(uint16_t port, const char *str) {
    while (*str) serial_putchar(port, *str++);
}

bool serial_received(uint16_t port) {
    return (inb(port + 5) & 0x01) != 0;
}

char serial_getchar(uint16_t port) {
    while (!serial_received(port));
    return (char)inb(port);
}

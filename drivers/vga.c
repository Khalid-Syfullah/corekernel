#include "vga.h"
#include <io.h>
#include <string.h>

#define VGA_CTRL_REG  0x3D4
#define VGA_DATA_REG  0x3D5

static uint16_t *vga_buf = (uint16_t *)VGA_MEMORY;
static uint8_t   vga_row;
static uint8_t   vga_col;
static uint8_t   vga_attr;  /* packed fg | (bg << 4) */

static inline uint16_t vga_entry(char c, uint8_t attr) {
    return (uint16_t)c | ((uint16_t)attr << 8);
}

static void hw_cursor_update(void) {
    uint16_t pos = vga_row * VGA_WIDTH + vga_col;
    outb(VGA_CTRL_REG, 0x0F); outb(VGA_DATA_REG, pos & 0xFF);
    outb(VGA_CTRL_REG, 0x0E); outb(VGA_DATA_REG, (pos >> 8) & 0xFF);
}

void vga_enable_cursor(uint8_t top, uint8_t bottom) {
    outb(VGA_CTRL_REG, 0x0A);
    outb(VGA_DATA_REG, (inb(VGA_DATA_REG) & 0xC0) | top);
    outb(VGA_CTRL_REG, 0x0B);
    outb(VGA_DATA_REG, (inb(VGA_DATA_REG) & 0xE0) | bottom);
}

void vga_disable_cursor(void) {
    outb(VGA_CTRL_REG, 0x0A);
    outb(VGA_DATA_REG, 0x20);
}

void vga_init(void) {
    vga_row  = 0;
    vga_col  = 0;
    vga_attr = (VGA_COLOR_BLACK << 4) | VGA_COLOR_LIGHT_GREY;
    vga_clear();
    vga_enable_cursor(14, 15);
}

void vga_clear(void) {
    uint16_t blank = vga_entry(' ', vga_attr);
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga_buf[i] = blank;
    vga_row = 0;
    vga_col = 0;
    hw_cursor_update();
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    vga_attr = (uint8_t)((bg << 4) | (fg & 0x0F));
}

void vga_scroll(void) {
    /* Move every row up by one */
    for (int row = 1; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            vga_buf[(row - 1) * VGA_WIDTH + col] =
                vga_buf[row * VGA_WIDTH + col];
        }
    }
    /* Clear last row */
    uint16_t blank = vga_entry(' ', vga_attr);
    for (int col = 0; col < VGA_WIDTH; col++)
        vga_buf[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = blank;
    vga_row = VGA_HEIGHT - 1;
}

void vga_put_char(char c) {
    if (c == '\n') {
        vga_col = 0;
        if (++vga_row >= VGA_HEIGHT) vga_scroll();
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\t') {
        vga_col = (uint8_t)((vga_col + 8) & ~7);
        if (vga_col >= VGA_WIDTH) {
            vga_col = 0;
            if (++vga_row >= VGA_HEIGHT) vga_scroll();
        }
    } else if (c == '\b') {
        if (vga_col > 0) {
            vga_col--;
            vga_buf[vga_row * VGA_WIDTH + vga_col] = vga_entry(' ', vga_attr);
        }
    } else {
        vga_buf[vga_row * VGA_WIDTH + vga_col] = vga_entry(c, vga_attr);
        if (++vga_col >= VGA_WIDTH) {
            vga_col = 0;
            if (++vga_row >= VGA_HEIGHT) vga_scroll();
        }
    }
    hw_cursor_update();
}

void vga_put_string(const char *str) {
    while (*str) vga_put_char(*str++);
}

void vga_set_cursor(uint8_t x, uint8_t y) {
    vga_col = x; vga_row = y;
    hw_cursor_update();
}

void vga_get_cursor(uint8_t *x, uint8_t *y) {
    *x = vga_col; *y = vga_row;
}

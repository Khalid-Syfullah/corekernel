typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;

#define VGA_BASE ((volatile uint16_t *)0xB8000)

static void vga_print(const char *s) {
    volatile uint16_t *vga = VGA_BASE;
    for (int i = 0; *s; s++, i++)
        vga[i] = (uint16_t)(0x0700 | (uint8_t)*s);
}

void kmain(void) {
    vga_print("Hello, Kernel!");
    for (;;)
        __asm__ volatile("hlt");
}

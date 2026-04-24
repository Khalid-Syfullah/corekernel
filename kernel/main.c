#include <multiboot.h>
#include <stdint.h>
#include <vga.h>
#include <serial.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <irq.h>
#include <timer.h>
#include <keyboard.h>
#include <pmm.h>
#include <paging.h>
#include <heap.h>
#include <printf.h>
#include <string.h>
#include <shell.h>

/* Kernel symbols from linker script */
extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

static void banner(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    kprintf("  ____               _  __                    _\n");
    kprintf(" / ___|___  _ __ ___ | |/ /___ _ __ _ __   ___| |\n");
    kprintf("| |   / _ \\| '__/ _ \\| ' // _ \\ '__| '_ \\ / _ \\ |\n");
    kprintf("| |__| (_) | | |  __/| . \\  __/ |  | | | |  __/ |\n");
    kprintf(" \\____\\___/|_|  \\___||_|\\_\\___|_|  |_| |_|\\___|_|\n");
    vga_set_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    kprintf("  x86 Research Kernel  v0.1.0  "
            __DATE__ "  " __TIME__ "\n\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

static void init_subsystem(const char *name, void (*fn)(void)) {
    vga_set_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    kprintf("  [ ");
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    kprintf("INIT");
    vga_set_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    kprintf(" ] %-24s", name);
    fn();
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    kprintf("OK\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

/* Wrapper for subsystems that need arguments */
static multiboot_info_t *g_mbi;

static void pmm_init_wrapper(void) {
    pmm_init(g_mbi,
             (uint32_t)&_kernel_start,
             (uint32_t)&_kernel_end);
}

void kernel_main(uint32_t magic, multiboot_info_t *mbi) {
    /* VGA must come first — we need output for everything else */
    vga_init();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        kprintf("FATAL: not booted by a multiboot-compliant loader (magic=0x%08x)\n", magic);
        __asm__ volatile("cli; hlt");
    }

    banner();
    kprintf("Kernel at 0x%08x - 0x%08x\n\n",
            (uint32_t)&_kernel_start, (uint32_t)&_kernel_end);

    g_mbi = mbi;

    init_subsystem("GDT",            gdt_init);
    init_subsystem("IDT",            idt_init);
    init_subsystem("ISR",            isr_init);
    init_subsystem("IRQ",            irq_init);
    init_subsystem("PIT Timer",      (void(*)(void))({ timer_init(TIMER_FREQ_HZ); }));
    init_subsystem("Keyboard (PS/2)", keyboard_init);
    init_subsystem("Serial (COM1)",  (void(*)(void))({ serial_init(SERIAL_COM1); }));
    init_subsystem("PMM",            pmm_init_wrapper);
    init_subsystem("Paging",         paging_init);
    init_subsystem("Heap",           heap_init);

    /* Enable interrupts — IRQs live from here */
    __asm__ volatile("sti");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    kprintf("\n  All subsystems initialized. Interrupts enabled.\n\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    /* Print memory summary */
    kprintf("  RAM: %u MiB total, %u MiB free\n",
            (pmm_total_pages() * 4) / 1024,
            (pmm_free_pages()  * 4) / 1024);
    kprintf("\n");

    serial_puts(SERIAL_COM1, "CoreKernel booted\n");

    /* Hand off to the interactive shell */
    shell_run();

    /* Should never reach here */
    __asm__ volatile("cli");
    while (1) __asm__ volatile("hlt");
}

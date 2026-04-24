#include "isr.h"
#include <vga.h>
#include <printf.h>
#include <string.h>

static isr_handler_t handlers[ISR_COUNT];

static const char *exception_msgs[32] = {
    "Division by Zero",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"
};

void isr_init(void) {
    memset(handlers, 0, sizeof(handlers));
}

void isr_register_handler(uint8_t num, isr_handler_t handler) {
    if (num < ISR_COUNT) handlers[num] = handler;
}

void isr_handler(registers_t *regs) {
    uint32_t num = regs->int_no;

    if (num < ISR_COUNT && handlers[num]) {
        handlers[num](regs);
        return;
    }

    /* Unhandled exception → kernel panic display */
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    kprintf("\n\n *** KERNEL EXCEPTION #%u: %s ***\n",
            num, num < 32 ? exception_msgs[num] : "Unknown");
    kprintf(" Error code : 0x%08x\n", regs->err_code);
    kprintf(" EIP=0x%08x  CS=0x%04x  EFLAGS=0x%08x\n",
            regs->eip, regs->cs, regs->eflags);
    kprintf(" EAX=0x%08x  EBX=0x%08x  ECX=0x%08x  EDX=0x%08x\n",
            regs->eax, regs->ebx, regs->ecx, regs->edx);
    kprintf(" ESI=0x%08x  EDI=0x%08x  EBP=0x%08x  ESP=0x%08x\n",
            regs->esi, regs->edi, regs->ebp, regs->esp);

    __asm__ volatile("cli; hlt");
    while (1) {}
}

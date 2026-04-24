#pragma once
#include "idt.h"

#define IRQ_COUNT 16

typedef void (*irq_handler_t)(registers_t *regs);

void irq_init(void);
void irq_register_handler(uint8_t irq, irq_handler_t handler);
void irq_unregister_handler(uint8_t irq);

/* Called from irq_common_stub in assembly */
void irq_handler(registers_t *regs);

/* PIC end-of-interrupt */
void irq_send_eoi(uint8_t irq);

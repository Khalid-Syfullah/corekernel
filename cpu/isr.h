#pragma once
#include "idt.h"

#define ISR_COUNT 32

typedef void (*isr_handler_t)(registers_t *regs);

void isr_init(void);
void isr_register_handler(uint8_t isr, isr_handler_t handler);

/* C-level dispatcher called from isr_common_stub */
void isr_handler(registers_t *regs);

#include "irq.h"
#include <io.h>
#include <string.h>

#define PIC_MASTER_CMD  0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD   0xA0
#define PIC_SLAVE_DATA  0xA1
#define PIC_EOI         0x20

static irq_handler_t handlers[IRQ_COUNT];

void irq_init(void) {
    memset(handlers, 0, sizeof(handlers));
}

void irq_register_handler(uint8_t irq, irq_handler_t handler) {
    if (irq < IRQ_COUNT) handlers[irq] = handler;
}

void irq_unregister_handler(uint8_t irq) {
    if (irq < IRQ_COUNT) handlers[irq] = NULL;
}

void irq_send_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC_SLAVE_CMD,  PIC_EOI);
    outb(PIC_MASTER_CMD, PIC_EOI);
}

/* IRQ vectors start at 32 */
void irq_handler(registers_t *regs) {
    uint8_t irq = (uint8_t)(regs->int_no - 32);

    if (irq < IRQ_COUNT && handlers[irq]) {
        handlers[irq](regs);
    }

    irq_send_eoi(irq);
}

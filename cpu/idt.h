#pragma once
#include <stdint.h>

#define IDT_ENTRIES 256

/* Gate type + attributes */
#define IDT_GATE_TASK       0x05
#define IDT_GATE_INT16      0x06
#define IDT_GATE_TRAP16     0x07
#define IDT_GATE_INT32      0x0E
#define IDT_GATE_TRAP32     0x0F

#define IDT_FLAG_PRESENT    0x80
#define IDT_FLAG_RING0      0x00
#define IDT_FLAG_RING3      0x60

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

/* CPU-pushed register state passed to ISR/IRQ handlers */
typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pusha */
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;            /* pushed by CPU */
} registers_t;

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t handler, uint16_t sel, uint8_t flags);

extern void idt_flush(uint32_t idt_ptr);

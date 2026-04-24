#include "gdt.h"
#include <string.h>

static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t   gdt_ptr;
static tss_entry_t tss;

static void gdt_set_entry(int idx, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t gran) {
    gdt[idx].base_low    = base & 0xFFFF;
    gdt[idx].base_mid    = (base >> 16) & 0xFF;
    gdt[idx].base_high   = (base >> 24) & 0xFF;
    gdt[idx].limit_low   = limit & 0xFFFF;
    gdt[idx].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[idx].access      = access;
}

static void tss_init(uint32_t kernel_ss, uint32_t kernel_esp) {
    uint32_t base  = (uint32_t)&tss;
    uint32_t limit = base + sizeof(tss_entry_t);

    gdt_set_entry(GDT_TSS, base, limit,
                  0x89,   /* present, ring0, 32-bit TSS available */
                  0x00);

    memset(&tss, 0, sizeof(tss_entry_t));
    tss.ss0  = kernel_ss;
    tss.esp0 = kernel_esp;
    tss.cs   = 0x0B;    /* user code segment | 3 */
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13; /* user data | 3 */
    tss.iomap_base = sizeof(tss_entry_t);
}

void gdt_init(void) {
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (uint32_t)&gdt;

    gdt_set_entry(GDT_NULL,        0, 0x00000000, 0x00, 0x00);
    gdt_set_entry(GDT_KERNEL_CODE, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* ring0 code */
    gdt_set_entry(GDT_KERNEL_DATA, 0, 0xFFFFFFFF, 0x92, 0xCF); /* ring0 data */
    gdt_set_entry(GDT_USER_CODE,   0, 0xFFFFFFFF, 0xFA, 0xCF); /* ring3 code */
    gdt_set_entry(GDT_USER_DATA,   0, 0xFFFFFFFF, 0xF2, 0xCF); /* ring3 data */

    tss_init(0x10, 0x0);  /* kernel data segment, stack set later */

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}

void gdt_set_kernel_stack(uint32_t stack) {
    tss.esp0 = stack;
}

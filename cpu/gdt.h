#pragma once
#include <stdint.h>

/* GDT segment indices */
#define GDT_NULL        0
#define GDT_KERNEL_CODE 1
#define GDT_KERNEL_DATA 2
#define GDT_USER_CODE   3
#define GDT_USER_DATA   4
#define GDT_TSS         5
#define GDT_ENTRIES     6

/* Access byte flags */
#define GDT_ACCESS_PRESENT   0x80
#define GDT_ACCESS_RING0     0x00
#define GDT_ACCESS_RING3     0x60
#define GDT_ACCESS_SYSTEM    0x10
#define GDT_ACCESS_EXEC      0x08
#define GDT_ACCESS_DC        0x04
#define GDT_ACCESS_RW        0x02
#define GDT_ACCESS_ACCESSED  0x01

/* Granularity byte flags */
#define GDT_FLAG_4K_GRAN     0x80
#define GDT_FLAG_32BIT       0x40

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

/* TSS — used for ring0 stack on privilege transitions */
typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;      /* ring-0 stack pointer */
    uint32_t ss0;       /* ring-0 stack segment */
    uint32_t esp1, ss1;
    uint32_t esp2, ss2;
    uint32_t cr3, eip, eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

void gdt_init(void);
void gdt_set_kernel_stack(uint32_t stack);

/* Assembly routines */
extern void gdt_flush(uint32_t gdt_ptr);
extern void tss_flush(void);

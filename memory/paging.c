#include "paging.h"
#include "pmm.h"
#include <isr.h>
#include <vga.h>
#include <printf.h>
#include <string.h>
#include <io.h>

/* Kernel page directory — 4 KiB aligned */
static page_dir_t kernel_page_dir __attribute__((aligned(PAGE_SIZE)));

/* Identity-map the first page tables */
#define IDENTITY_PAGES 1024
static page_table_t low_page_table __attribute__((aligned(PAGE_SIZE)));

static void page_fault_handler(registers_t *regs) {
    uint32_t fault_addr = read_cr2();
    bool present   = !(regs->err_code & 0x1);
    bool rw        =  (regs->err_code & 0x2) != 0;
    bool user_mode =  (regs->err_code & 0x4) != 0;

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    kprintf("\n *** PAGE FAULT ***\n");
    kprintf(" Address   : 0x%08x\n", fault_addr);
    kprintf(" Cause     : %s%s%s\n",
            present   ? "protection-violation " : "non-present-page ",
            rw        ? "write " : "read ",
            user_mode ? "(user) " : "(kernel) ");
    kprintf(" EIP       : 0x%08x\n", regs->eip);

    __asm__ volatile("cli; hlt");
    while (1) {}
}

void paging_init(void) {
    /* Register page fault handler (ISR 14) */
    isr_register_handler(14, page_fault_handler);

    /* Identity-map 0..4MB (kernel code + low memory) */
    for (uint32_t i = 0; i < IDENTITY_PAGES; i++) {
        low_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;
    }
    kernel_page_dir[0] = ((uint32_t)low_page_table) | PAGE_PRESENT | PAGE_WRITABLE;

    /* Also map 0..4MB at 0xC0000000 (higher-half stub) */
    kernel_page_dir[768] = ((uint32_t)low_page_table) | PAGE_PRESENT | PAGE_WRITABLE;

    /* Load page directory and enable paging */
    write_cr3((uint32_t)kernel_page_dir);
    write_cr0(read_cr0() | 0x80000000);
}

bool paging_map(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t dir_idx   = virt >> 22;
    uint32_t table_idx = (virt >> 12) & 0x3FF;

    page_table_t *table;
    if (!(kernel_page_dir[dir_idx] & PAGE_PRESENT)) {
        table = (page_table_t *)pmm_alloc_page();
        if (!table) return false;
        memset(table, 0, PAGE_SIZE);
        kernel_page_dir[dir_idx] = (uint32_t)table | PAGE_PRESENT | PAGE_WRITABLE;
    } else {
        table = (page_table_t *)(kernel_page_dir[dir_idx] & ~0xFFF);
    }

    (*table)[table_idx] = (phys & ~0xFFF) | (flags & 0xFFF) | PAGE_PRESENT;
    invlpg(virt);
    return true;
}

void paging_unmap(uint32_t virt) {
    uint32_t dir_idx   = virt >> 22;
    uint32_t table_idx = (virt >> 12) & 0x3FF;
    if (!(kernel_page_dir[dir_idx] & PAGE_PRESENT)) return;
    page_table_t *table = (page_table_t *)(kernel_page_dir[dir_idx] & ~0xFFF);
    (*table)[table_idx] = 0;
    invlpg(virt);
}

uint32_t paging_virt_to_phys(uint32_t virt) {
    uint32_t dir_idx   = virt >> 22;
    uint32_t table_idx = (virt >> 12) & 0x3FF;
    if (!(kernel_page_dir[dir_idx] & PAGE_PRESENT)) return 0;
    page_table_t *table = (page_table_t *)(kernel_page_dir[dir_idx] & ~0xFFF);
    if (!((*table)[table_idx] & PAGE_PRESENT)) return 0;
    return ((*table)[table_idx] & ~0xFFF) | (virt & 0xFFF);
}

bool paging_is_mapped(uint32_t virt) {
    return paging_virt_to_phys(virt) != 0;
}

void paging_switch_dir(page_dir_t *dir) {
    write_cr3((uint32_t)dir);
}

page_dir_t *paging_get_kernel_dir(void) {
    return &kernel_page_dir;
}

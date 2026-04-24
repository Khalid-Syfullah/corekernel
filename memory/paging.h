#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE       4096
#define PAGE_ENTRIES    1024

/* Page entry flags */
#define PAGE_PRESENT    0x001
#define PAGE_WRITABLE   0x002
#define PAGE_USER       0x004
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040
#define PAGE_4MB        0x080

typedef uint32_t page_t;
typedef uint32_t page_table_t[PAGE_ENTRIES];
typedef uint32_t page_dir_t[PAGE_ENTRIES];

void paging_init(void);
bool paging_map(uint32_t virt, uint32_t phys, uint32_t flags);
void paging_unmap(uint32_t virt);
uint32_t paging_virt_to_phys(uint32_t virt);
bool paging_is_mapped(uint32_t virt);
void paging_switch_dir(page_dir_t *dir);
page_dir_t *paging_get_kernel_dir(void);

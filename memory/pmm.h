#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <multiboot.h>

#define PAGE_SIZE       4096
#define PAGES_PER_BLOCK 32

/* Total physical pages manageable (128 MB / 4 KB = 32768 pages) */
#define PMM_MAX_PAGES   (128 * 1024 * 1024 / PAGE_SIZE)

void pmm_init(multiboot_info_t *mbi, uint32_t kernel_start, uint32_t kernel_end);
void *pmm_alloc_page(void);
void  pmm_free_page(void *page);
bool  pmm_is_page_free(uint32_t frame);

uint32_t pmm_total_pages(void);
uint32_t pmm_free_pages(void);
uint32_t pmm_used_pages(void);

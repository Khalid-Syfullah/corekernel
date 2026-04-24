#include "pmm.h"
#include <string.h>

/* Bitmap: 1 bit per 4 KiB page — 1 = free, 0 = used */
static uint32_t bitmap[PMM_MAX_PAGES / PAGES_PER_BLOCK];
static uint32_t total_pages = 0;
static uint32_t free_pages_count = 0;

static inline void bitmap_set(uint32_t frame) {
    bitmap[frame / PAGES_PER_BLOCK] |= (1U << (frame % PAGES_PER_BLOCK));
}

static inline void bitmap_clear(uint32_t frame) {
    bitmap[frame / PAGES_PER_BLOCK] &= ~(1U << (frame % PAGES_PER_BLOCK));
}

static inline bool bitmap_test(uint32_t frame) {
    return (bitmap[frame / PAGES_PER_BLOCK] & (1U << (frame % PAGES_PER_BLOCK))) != 0;
}

void pmm_init(multiboot_info_t *mbi, uint32_t kernel_start, uint32_t kernel_end) {
    /* Start with everything unavailable */
    memset(bitmap, 0, sizeof(bitmap));
    free_pages_count = 0;

    if (!(mbi->flags & MULTIBOOT_FLAG_MMAP)) return;

    /* Mark available regions from BIOS memory map */
    multiboot_mmap_entry_t *entry = (multiboot_mmap_entry_t *)mbi->mmap_addr;
    multiboot_mmap_entry_t *end   = (multiboot_mmap_entry_t *)(mbi->mmap_addr + mbi->mmap_length);

    while (entry < end) {
        if (entry->type == 1) {  /* available */
            uint32_t frame_start = (uint32_t)(entry->addr / PAGE_SIZE);
            uint32_t frame_count = (uint32_t)(entry->len  / PAGE_SIZE);
            for (uint32_t i = frame_start; i < frame_start + frame_count; i++) {
                if (i < PMM_MAX_PAGES) {
                    bitmap_set(i);
                    free_pages_count++;
                    total_pages++;
                }
            }
        }
        entry = (multiboot_mmap_entry_t *)((uint8_t *)entry + entry->size + 4);
    }

    /* Lock the first page (null pointer protection) */
    bitmap_clear(0);
    free_pages_count--;

    /* Lock kernel pages */
    uint32_t ks = kernel_start / PAGE_SIZE;
    uint32_t ke = (kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = ks; i <= ke; i++) {
        if (bitmap_test(i)) { bitmap_clear(i); free_pages_count--; }
    }

    /* Lock PMM bitmap itself */
    uint32_t bm_start = (uint32_t)bitmap / PAGE_SIZE;
    uint32_t bm_end   = ((uint32_t)bitmap + sizeof(bitmap) + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = bm_start; i <= bm_end; i++) {
        if (bitmap_test(i)) { bitmap_clear(i); free_pages_count--; }
    }
}

void *pmm_alloc_page(void) {
    for (uint32_t block = 0; block < PMM_MAX_PAGES / PAGES_PER_BLOCK; block++) {
        if (bitmap[block] == 0) continue;
        for (uint32_t bit = 0; bit < PAGES_PER_BLOCK; bit++) {
            if (bitmap[block] & (1U << bit)) {
                uint32_t frame = block * PAGES_PER_BLOCK + bit;
                bitmap_clear(frame);
                free_pages_count--;
                return (void *)(uintptr_t)(frame * PAGE_SIZE);
            }
        }
    }
    return NULL;  /* out of memory */
}

void pmm_free_page(void *page) {
    uint32_t frame = (uint32_t)(uintptr_t)page / PAGE_SIZE;
    if (!bitmap_test(frame)) {
        bitmap_set(frame);
        free_pages_count++;
    }
}

bool pmm_is_page_free(uint32_t frame) { return bitmap_test(frame); }
uint32_t pmm_total_pages(void)        { return total_pages; }
uint32_t pmm_free_pages(void)         { return free_pages_count; }
uint32_t pmm_used_pages(void)         { return total_pages - free_pages_count; }

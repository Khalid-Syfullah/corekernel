#include "heap.h"
#include "paging.h"
#include "pmm.h"
#include <string.h>

/* Free-list heap allocator (first-fit) */

typedef struct block_header {
    uint32_t magic;
    size_t   size;      /* usable bytes after header */
    bool     free;
    struct block_header *next;
    struct block_header *prev;
} block_header_t;

#define HEADER_SZ sizeof(block_header_t)
#define MIN_SPLIT 32  /* don't split if remaining < this */

static block_header_t *heap_head = NULL;
static uintptr_t       heap_brk  = 0;

/* Extend heap by at least `bytes` (page-aligned) */
static bool heap_grow(size_t bytes) {
    size_t pages = (bytes + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t i = 0; i < pages; i++) {
        void *phys = pmm_alloc_page();
        if (!phys) return false;
        if (!paging_map(heap_brk, (uint32_t)phys, PAGE_PRESENT | PAGE_WRITABLE))
            return false;
        heap_brk += PAGE_SIZE;
    }
    return true;
}

void heap_init(void) {
    heap_brk = HEAP_START;
    if (!heap_grow(HEAP_INITIAL_SZ)) return;

    heap_head = (block_header_t *)HEAP_START;
    heap_head->magic = HEAP_MAGIC;
    heap_head->size  = HEAP_INITIAL_SZ - HEADER_SZ;
    heap_head->free  = true;
    heap_head->next  = NULL;
    heap_head->prev  = NULL;
}

static void split_block(block_header_t *blk, size_t size) {
    if (blk->size < size + HEADER_SZ + MIN_SPLIT) return;
    block_header_t *split = (block_header_t *)((uint8_t *)blk + HEADER_SZ + size);
    split->magic = HEAP_MAGIC;
    split->size  = blk->size - size - HEADER_SZ;
    split->free  = true;
    split->next  = blk->next;
    split->prev  = blk;
    if (blk->next) blk->next->prev = split;
    blk->next = split;
    blk->size = size;
}

static void coalesce(block_header_t *blk) {
    /* Merge with next */
    if (blk->next && blk->next->free) {
        blk->size += HEADER_SZ + blk->next->size;
        blk->next  = blk->next->next;
        if (blk->next) blk->next->prev = blk;
    }
    /* Merge with previous */
    if (blk->prev && blk->prev->free) {
        blk->prev->size += HEADER_SZ + blk->size;
        blk->prev->next  = blk->next;
        if (blk->next) blk->next->prev = blk->prev;
    }
}

void *kmalloc(size_t size) {
    if (!size) return NULL;
    size = (size + 7) & ~7;   /* 8-byte alignment */

    block_header_t *cur = heap_head;
    while (cur) {
        if (cur->free && cur->size >= size) {
            split_block(cur, size);
            cur->free = false;
            return (void *)((uint8_t *)cur + HEADER_SZ);
        }
        if (!cur->next) {
            /* Grow heap */
            size_t grow = size + HEADER_SZ;
            if (!heap_grow(grow)) return NULL;
            block_header_t *ext = (block_header_t *)heap_brk - 1;
            /* Re-scan from end of existing list after growing */
            cur->next = (block_header_t *)heap_brk - 1;
            continue;
        }
        cur = cur->next;
    }
    return NULL;
}

void *kmalloc_aligned(size_t size, size_t align) {
    /* Simple approach: over-allocate to guarantee alignment */
    void *raw = kmalloc(size + align + HEADER_SZ);
    if (!raw) return NULL;
    uintptr_t aligned = ((uintptr_t)raw + align - 1) & ~(align - 1);
    return (void *)aligned;
}

void *kcalloc(size_t count, size_t size) {
    void *p = kmalloc(count * size);
    if (p) memset(p, 0, count * size);
    return p;
}

void *krealloc(void *ptr, size_t new_size) {
    if (!ptr)     return kmalloc(new_size);
    if (!new_size) { kfree(ptr); return NULL; }

    block_header_t *blk = (block_header_t *)((uint8_t *)ptr - HEADER_SZ);
    if (blk->size >= new_size) return ptr;

    void *new_ptr = kmalloc(new_size);
    if (!new_ptr) return NULL;
    memcpy(new_ptr, ptr, blk->size);
    kfree(ptr);
    return new_ptr;
}

void kfree(void *ptr) {
    if (!ptr) return;
    block_header_t *blk = (block_header_t *)((uint8_t *)ptr - HEADER_SZ);
    if (blk->magic != HEAP_MAGIC) return;  /* invalid / double-free guard */
    blk->free = true;
    coalesce(blk);
}

size_t heap_used(void) {
    size_t used = 0;
    for (block_header_t *b = heap_head; b; b = b->next)
        if (!b->free) used += b->size;
    return used;
}

size_t heap_available(void) {
    size_t avail = 0;
    for (block_header_t *b = heap_head; b; b = b->next)
        if (b->free) avail += b->size;
    return avail;
}

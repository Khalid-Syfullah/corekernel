#pragma once
#include <stdint.h>
#include <stddef.h>

/* Kernel heap starts at 4 MB */
#define HEAP_START      0x00400000
#define HEAP_INITIAL_SZ 0x00100000   /* 1 MiB initial */
#define HEAP_MAX_SZ     0x02000000   /* 32 MiB max */
#define HEAP_MAGIC      0xDEADC0DE

void heap_init(void);
void *kmalloc(size_t size);
void *kmalloc_aligned(size_t size, size_t align);
void *kcalloc(size_t count, size_t size);
void *krealloc(void *ptr, size_t new_size);
void  kfree(void *ptr);

size_t heap_used(void);
size_t heap_available(void);

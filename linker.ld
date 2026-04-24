/* CoreKernel Linker Script — i686 ELF, multiboot-compatible */

ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

SECTIONS {
    /* Load at 1 MiB — standard for multiboot kernels */
    . = 1M;

    _kernel_start = .;

    /* Multiboot header must be within first 8 KiB */
    .multiboot ALIGN(4) : {
        *(.multiboot)
    }

    /* Code */
    .text ALIGN(4K) : {
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data ALIGN(4K) : {
        *(.data)
        *(.data.*)
    }

    /* Zero-initialized data */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    _kernel_end = .;

    /* Discard debug and comment sections */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
    }
}

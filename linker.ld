ENTRY(_start)

SECTIONS {
    . = 1M;

    .text   : { *(.multiboot) *(.text*) }
    .rodata : { *(.rodata*) }

    . = ALIGN(4096);
    .data   : { *(.data*) }
    .bss    : { *(.bss*) *(COMMON) }
}

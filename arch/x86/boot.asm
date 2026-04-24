; CoreKernel — Multiboot entry point
; Conforms to Multiboot Specification 0.96

MBOOT_PAGE_ALIGN    equ 1 << 0
MBOOT_MEM_INFO      equ 1 << 1
MBOOT_USE_GFX       equ 0
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

KERNEL_STACK_SIZE   equ 0x4000          ; 16 KiB

section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC
    dd MBOOT_HEADER_FLAGS
    dd MBOOT_CHECKSUM

section .bss
align 16
global kernel_stack_bottom
kernel_stack_bottom:
    resb KERNEL_STACK_SIZE
global kernel_stack_top
kernel_stack_top:

section .text
bits 32
global _start
extern kernel_main

_start:
    ; Set up the stack
    mov     esp, kernel_stack_top

    ; Clear EFLAGS
    push    0
    popf

    ; Push multiboot info: magic (eax) and info pointer (ebx)
    push    ebx
    push    eax

    ; Call the C kernel entry point
    call    kernel_main

    ; Should never reach here — hang forever
    cli
.hang:
    hlt
    jmp     .hang

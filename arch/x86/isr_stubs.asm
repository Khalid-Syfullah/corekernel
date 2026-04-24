; ISR and IRQ stub definitions
; All stubs save CPU state, call the C handler, restore state.

global isr_common_stub
global irq_common_stub

extern isr_handler
extern irq_handler

; Macro: ISR that pushes a dummy error code (CPU doesn't push one)
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push    dword 0             ; dummy error code
    push    dword %1            ; interrupt number
    jmp     isr_common_stub
%endmacro

; Macro: ISR where CPU already pushes an error code
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push    dword %1
    jmp     isr_common_stub
%endmacro

; Macro: IRQ stub (hardware interrupts 32-47)
%macro IRQ 2
global irq%1
irq%1:
    cli
    push    dword 0
    push    dword %2
    jmp     irq_common_stub
%endmacro

; CPU exceptions 0-31
ISR_NOERRCODE  0    ; Division by zero
ISR_NOERRCODE  1    ; Debug
ISR_NOERRCODE  2    ; NMI
ISR_NOERRCODE  3    ; Breakpoint
ISR_NOERRCODE  4    ; Overflow
ISR_NOERRCODE  5    ; Bound range exceeded
ISR_NOERRCODE  6    ; Invalid opcode
ISR_NOERRCODE  7    ; Device not available
ISR_ERRCODE    8    ; Double fault
ISR_NOERRCODE  9    ; Coprocessor segment overrun
ISR_ERRCODE   10    ; Invalid TSS
ISR_ERRCODE   11    ; Segment not present
ISR_ERRCODE   12    ; Stack-segment fault
ISR_ERRCODE   13    ; General protection fault
ISR_ERRCODE   14    ; Page fault
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; x87 FPU error
ISR_ERRCODE   17    ; Alignment check
ISR_NOERRCODE 18    ; Machine check
ISR_NOERRCODE 19    ; SIMD floating-point exception
ISR_NOERRCODE 20    ; Virtualization exception
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE   30    ; Security exception
ISR_NOERRCODE 31

; Hardware IRQs (remapped to 32-47)
IRQ  0, 32
IRQ  1, 33
IRQ  2, 34
IRQ  3, 35
IRQ  4, 36
IRQ  5, 37
IRQ  6, 38
IRQ  7, 39
IRQ  8, 40
IRQ  9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; System call (int 0x80)
ISR_NOERRCODE 128

; ---- Common ISR stub ----
isr_common_stub:
    pusha                       ; push eax, ecx, edx, ebx, esp, ebp, esi, edi

    mov     ax, ds
    push    eax                 ; save data segment

    mov     ax, 0x10            ; kernel data segment
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    push    esp                 ; pass pointer to registers_t
    call    isr_handler
    add     esp, 4

    pop     eax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    popa
    add     esp, 8              ; clean up error code and ISR number
    iret

; ---- Common IRQ stub ----
irq_common_stub:
    pusha

    mov     ax, ds
    push    eax

    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    push    esp
    call    irq_handler
    add     esp, 4

    pop     eax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    popa
    add     esp, 8
    iret

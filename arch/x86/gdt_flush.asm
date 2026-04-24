; Load the GDT and reload all segment registers

global gdt_flush
global tss_flush

gdt_flush:
    mov     eax, [esp + 4]      ; pointer to GDT descriptor
    lgdt    [eax]

    ; Reload code segment via far jump
    jmp     0x08:.reload_cs
.reload_cs:
    ; Reload data segments
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    ret

tss_flush:
    mov     ax, 0x2B            ; TSS selector (index 5, RPL 3)
    ltr     ax
    ret

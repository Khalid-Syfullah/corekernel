# CoreKernel Makefile
# Cross-compiler target: i686-elf
# Assembler: nasm

TARGET   := i686-elf
CC       := $(TARGET)-gcc
LD       := $(TARGET)-gcc
AS       := nasm

KERNEL   := corekernel.elf
ISO      := corekernel.iso

CFLAGS   := -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
            -fno-stack-protector -fno-builtin \
            -Iinclude -Ilib -Icpu -Idrivers -Imemory -Ikernel -Ishell

LDFLAGS  := -ffreestanding -nostdlib -lgcc -T linker.ld
ASFLAGS  := -f elf32

# Source files
ASM_SRCS := arch/x86/boot.asm       \
            arch/x86/gdt_flush.asm   \
            arch/x86/idt_flush.asm   \
            arch/x86/isr_stubs.asm

C_SRCS   := kernel/main.c            \
            cpu/gdt.c                 \
            cpu/idt.c                 \
            cpu/isr.c                 \
            cpu/irq.c                 \
            drivers/vga.c             \
            drivers/timer.c           \
            drivers/keyboard.c        \
            drivers/serial.c          \
            memory/pmm.c              \
            memory/paging.c           \
            memory/heap.c             \
            lib/string.c              \
            lib/printf.c              \
            shell/shell.c

# Object files
ASM_OBJS := $(ASM_SRCS:.asm=.asm.o)
C_OBJS   := $(C_SRCS:.c=.o)
ALL_OBJS := $(ASM_OBJS) $(C_OBJS)

.PHONY: all clean iso run debug check

all: $(KERNEL)

$(KERNEL): $(ALL_OBJS) linker.ld
	$(LD) -o $@ $(ALL_OBJS) $(LDFLAGS)
	@echo "  LD    $@"
	@size $@

%.asm.o: %.asm
	@echo "  AS    $<"
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	@echo "  CC    $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Build bootable ISO with GRUB
iso: $(KERNEL)
	@mkdir -p iso/boot/grub
	@cp $(KERNEL) iso/boot/
	@printf 'set timeout=0\nset default=0\nmenuentry "CoreKernel" {\n  multiboot /boot/$(KERNEL)\n  boot\n}\n' \
		> iso/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) iso 2>/dev/null
	@echo "  ISO   $(ISO)"

# Run in QEMU (requires iso target first)
run: iso
	qemu-system-i386 -cdrom $(ISO) -serial stdio -m 128M

# Run kernel ELF directly (faster, no ISO needed)
run-elf: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -serial stdio -m 128M

# Debug with GDB
debug: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -serial stdio -m 128M \
	  -s -S &
	$(TARGET)-gdb -ex "target remote :1234" \
	              -ex "symbol-file $(KERNEL)"

# Verify cross-compiler is present
check:
	@which $(CC) > /dev/null 2>&1 || \
	  (echo "ERROR: $(CC) not found. Build a cross-compiler or install i686-elf-gcc." && exit 1)
	@which $(AS) > /dev/null 2>&1 || \
	  (echo "ERROR: nasm not found. Install with: brew install nasm / apt install nasm" && exit 1)
	@echo "Toolchain OK"

clean:
	@find . -name '*.o' -delete
	@rm -f $(KERNEL) $(ISO)
	@rm -rf iso/
	@echo "  CLEAN"

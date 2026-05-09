CC     = gcc
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -nostdinc

all: corekernel.elf

corekernel.elf: boot.o kernel.o
	$(CC) -m32 -T linker.ld -o $@ -ffreestanding -O2 -nostdlib -no-pie $^

boot.o: boot.S
	$(CC) -m32 -c $< -o $@

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o corekernel.elf

.PHONY: all clean

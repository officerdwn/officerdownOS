# officerdownOS Makefile — no auto-ISO on `make`

CC = gcc
LD = ld
CFLAGS = -m32 -ffreestanding -fno-stack-protector -O2 -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld

SRC = kernel.c kernel_start.asm
OBJ = kernel.o ks.o

ISO_DIR = iso
KERNEL_BIN = kernel.bin
ISO = officerdownOS.iso

.PHONY: all iso run clean

all: $(KERNEL_BIN)

ks.o: kernel_start.asm
	nasm -f elf kernel_start.asm -o ks.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

$(KERNEL_BIN): $(OBJ)
	$(LD) $(LDFLAGS) -o $(KERNEL_BIN) $(OBJ)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	cp boot/grub/grub.cfg $(ISO_DIR)/boot/grub/

iso: $(KERNEL_BIN)
	grub-mkrescue -o $(ISO) $(ISO_DIR) --compress=xz

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

clean:
	rm -f *.o *.bin *.iso
	rm -rf $(ISO_DIR)


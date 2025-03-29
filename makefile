# officerdownOS Makefile (native gcc version)

CC = gcc
LD = ld
CFLAGS = -m32 -ffreestanding -fno-stack-protector -O2 -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld

SRC = kernel.c kernel_start.asm
OBJ = kernel.o ks.o

ISO_DIR = iso
KERNEL_BIN = kernel.bin
ISO = officerdownOS.iso

all: $(ISO)

ks.o: kernel_start.asm
	nasm -f elf kernel_start.asm -o ks.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

$(KERNEL_BIN): $(OBJ)
	$(LD) $(LDFLAGS) -o $(KERNEL_BIN) $(OBJ)

$(ISO): $(KERNEL_BIN)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	cp boot/grub/grub.cfg $(ISO_DIR)/boot/grub/
	grub-mkrescue -o $(ISO) $(ISO_DIR)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

clean:
	rm -f *.o *.bin *.iso
	rm -rf $(ISO_DIR)


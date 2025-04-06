# Compiler and linker settings
CC = gcc
AS = nasm
LD = ld
CFLAGS = -m32 -ffreestanding -fno-stack-protector -O2 -Wall -Wextra
ASFLAGS = -f elf
LDFLAGS = -m elf_i386 -T linker.ld

# Object files
OBJS = kernel.o ks.o

# ISO filename
ISO_NAME = officerdownOS.iso

# Default target
.PHONY: all
all: kernel.bin

# Kernel binary
kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile assembly files
ks.o: kernel_start.asm
	$(AS) $(ASFLAGS) $< -o $@

# ISO image
$(ISO_NAME): kernel.bin
	@echo "Building ISO image..."
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/
	cp boot/grub/grub.cfg iso/boot/grub/
	grub-mkrescue -o $(ISO_NAME) iso

# Build ISO image
.PHONY: iso
iso: $(ISO_NAME)

# Run with kernel binary directly
.PHONY: run-kernel
run-kernel: kernel.bin
	@echo "Running with kernel binary..."
	qemu-system-i386 -kernel kernel.bin

# Run with ISO
.PHONY: run-iso
run-iso: $(ISO_NAME)
	@echo "Running with ISO image..."
	qemu-system-i386 -cdrom $(ISO_NAME)

# Run target (configurable)
.PHONY: run
run:
ifeq ($(iso),1)
	@$(MAKE) run-iso
else
	@$(MAKE) run-kernel
endif

# Clean up
.PHONY: clean
clean:
	rm -f *.o *.bin *.iso
	rm -rf iso
	@echo "All build artifacts removed."

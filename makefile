all:
	nasm -f elf kernel_start.asm -o ks.o
	gcc -m32 -fno-stack-protector -c kernel.c -o kernel.o
	ld -melf_i386 -T linker.ld -o kernel.bin ks.o kernel.o
clean:
	rm -f *.o
	rm -f kernel.bin
run:
	qemu-system-i386 -kernel kernel.bin


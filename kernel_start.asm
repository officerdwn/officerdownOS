; Multiboot header defines
MBALIGN     equ 1 << 0                  ; align loaded modules on page boundaries
MEMINFO     equ 1 << 1                  ; provide memory map
FLAGS       equ MBALIGN | MEMINFO       ; this is the Multiboot 'flag' field
MAGIC       equ 0x1BADB002              ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)        ; checksum of above to prove we are multiboot

; Declare global symbols
global start
extern kernel_main

; Put the Multiboot header in its own dedicated section
section .text.boot
align 4
multiboot_header:
    dd MAGIC                            ; Magic number
    dd FLAGS                            ; Flags
    dd CHECKSUM                         ; Checksum

; ---------------------- Multiboot2 header for UEFI --------------------------
MB2_MAGIC      equ 0xE85250D6
MB2_ARCH_I386  equ 0
MB2_LENGTH     equ mb2_header_end - mb2_header_start
MB2_CHECKSUM   equ -(MB2_MAGIC + MB2_ARCH_I386 + MB2_LENGTH)

align 8
mb2_header_start:
    dd MB2_MAGIC
    dd MB2_ARCH_I386
    dd MB2_LENGTH
    dd MB2_CHECKSUM
    ; Entry address tag
    dw 3                  ; type = entry address
    dw 0                  ; flags
    dd 12                 ; size
    dd start              ; entry point
    dd 0
    ; End tag
    dw 0
    dw 0
    dd 8
mb2_header_end:

; The kernel entry point
section .text
start:
    ; Set up stack pointer
    mov esp, stack_top

    ; Call the kernel main function
    call kernel_main
    
    ; Hang if kernel_main unexpectedly returns
    cli
.hang:
    hlt
    jmp .hang

; Allocate a stack
section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KiB
stack_top:

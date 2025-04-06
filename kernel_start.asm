; Multiboot header defines
MBALIGN     equ 1 << 0                  ; align loaded modules on page boundaries
MEMINFO     equ 1 << 1                  ; provide memory map
FLAGS       equ MBALIGN | MEMINFO       ; this is the Multiboot 'flag' field
MAGIC       equ 0x1BADB002              ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)        ; checksum of above to prove we are multiboot

; Declare global symbols
global start
extern kernel_main

; Put the multiboot header in its own dedicated section
section .text.boot
align 4
multiboot_header:
    dd MAGIC                            ; Magic number
    dd FLAGS                            ; Flags
    dd CHECKSUM                         ; Checksum

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

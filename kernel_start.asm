section .multiboot
align 4

    dd 0x1BADB002
    dd 0x00000003
    dd -(0x1BADB002 + 0x00000003)

section .text
global start
extern kernel_main

start:
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang


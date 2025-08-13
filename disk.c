#include "disk.h"
#include "kernel.h"

static inline unsigned short inw(unsigned short port) {
    unsigned short value;
    __asm__ __volatile__("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outw(unsigned short port, unsigned short value) {
    __asm__ __volatile__("outw %0, %1" : : "a"(value), "Nd"(port));
}

void read_sector(unsigned int lba, unsigned char *buffer) {
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)(lba & 0xFF));
    outb(0x1F4, (unsigned char)((lba >> 8) & 0xFF));
    outb(0x1F5, (unsigned char)((lba >> 16) & 0xFF));
    outb(0x1F7, 0x20);

    while (inb(0x1F7) & 0x80);
    while (!(inb(0x1F7) & 0x08));

    for (int i = 0; i < 256; i++) {
        ((unsigned short*)buffer)[i] = inw(0x1F0);
    }
}

void write_sector(unsigned int lba, const unsigned char *buffer) {
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)(lba & 0xFF));
    outb(0x1F4, (unsigned char)((lba >> 8) & 0xFF));
    outb(0x1F5, (unsigned char)((lba >> 16) & 0xFF));
    outb(0x1F7, 0x30);

    while (inb(0x1F7) & 0x80);
    while (!(inb(0x1F7) & 0x08));

    for (int i = 0; i < 256; i++) {
        outw(0x1F0, ((const unsigned short*)buffer)[i]);
    }
}


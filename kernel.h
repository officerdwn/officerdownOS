#ifndef KERNEL_H
#define KERNEL_H

// Function Prototypes
void kernel_main();
void actualizar_cursor(int row, int col);
void limpiar_pantalla();
unsigned int imprimir_pantalla(char *string);
char get_input();  // ✅ Ensure `get_input()` is declared
void get_string(char *buffer, int max_length); // ✅ Add `get_string()`
int strcmp(const char *s1, const char *s2);  // ✅ Add `strcmp()`

// Port I/O functions
static inline unsigned char inb(unsigned short port) {
    unsigned char value;
    __asm__ __volatile__("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

extern char scancode_to_ascii[128];

#endif



#ifndef KERNEL_H
#define KERNEL_H

// Function Prototypes
void kernel_main();
void update_cursor(int row, int col);
void clear_screen();
unsigned int print_to_screen(char *string);
char get_input();
void get_string(char *buffer, int max_length);
int string_compare(const char *s1, const char *s2);
int string_to_int(const char *str);
void int_to_string(int num, char *str, int base);

// Port I/O functions
static inline unsigned char inb(unsigned short port) {
    unsigned char value;
    __asm__ __volatile__("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Scancode lookup table
extern char scancode_to_ascii[128];
extern char shifted_scancode_to_ascii[128];

#endif

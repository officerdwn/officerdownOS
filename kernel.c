#include "kernel.h"
#include "screen.h"

void display_ui(void);
void display_about(void);
void display_calculator(void);

// Global state
int shift = 0;
int caps_lock = 0;
int previous_result = 0;

#define TEXT_WHITE 0x07
#define KEYBOARD_PORT 0x60
#define MAX_LINEAS 25

char *video_memory = (char *)0xb8000;
unsigned int cursor_line = 0;

// ------------------------ Keyboard Mappings (Scancode to ASCII) ------------------------
char scancode_to_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

char shifted_scancode_to_ascii[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

// ------------------------ Utility Functions (atoi, itoa, strcmp) ------------------------

int atoi(const char *str) {
    int result = 0, sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    }
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result * sign;
}

void itoa(int num, char *str, int base) {
    int i = 0, is_negative = 0;
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }
    while (num > 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'A' : rem + '0';
        num /= base;
    }
    if (is_negative) {
        str[i++] = '-';
    }
    str[i] = '\0';

    int start = 0, end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start++] = str[end];
        str[end--] = temp;
    }
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

// ------------------------ Core Kernel Logic (Main Loop, Screen Ops) ------------------------

void kernel_main() {
    clear_screen();
    while (1) {
        display_ui();
    }
}

void clear_screen() {
    for (unsigned int i = 0; i < (80 * 25 * 2); i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = TEXT_WHITE;
    }
    cursor_line = 0;
}

void scroll_screen() {
    for (int row = 1; row < MAX_LINEAS; row++) {
        for (int col = 0; col < 80; col++) {
            int from = (row * 80 + col) * 2;
            int to = ((row - 1) * 80 + col) * 2;
            video_memory[to] = video_memory[from];
            video_memory[to + 1] = video_memory[from + 1];
        }
    }

    // Clear last line
    for (int col = 0; col < 80; col++) {
        int pos = ((MAX_LINEAS - 1) * 80 + col) * 2;
        video_memory[pos] = ' ';
        video_memory[pos + 1] = TEXT_WHITE;
    }

    cursor_line = MAX_LINEAS - 1;
}

unsigned int print_to_screen(char *string) {
    int col = 0; // 🔧 Reset every time

    while (*string != 0) {
        if (*string == '\n') {
            cursor_line++;
            col = 0;
            if (cursor_line >= MAX_LINEAS) {
                scroll_screen();
            }
        } else {
            int pos = (cursor_line * 80 + col) * 2;
            video_memory[pos] = *string;
            video_memory[pos + 1] = TEXT_WHITE;
            col++;

            if (col >= 80) {
                cursor_line++;
                col = 0;
                if (cursor_line >= MAX_LINEAS) {
                    scroll_screen();
                }
            }
        }

        string++;
    }

    update_cursor(cursor_line, col);
    return 1;
}





void update_cursor(int row, int col) {
    unsigned short position = (row * 80) + col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

void get_string(char *buffer, int max_length) {
    int index = 0;
    static unsigned int col = 0;

    if (cursor_line >= MAX_LINEAS) scroll_screen();
    int row = cursor_line;
    col = 0;

    update_cursor(row, col);

    while (1) {
        char key = get_input();

        if (key == '\n') {
            buffer[index] = '\0';
            cursor_line++;
            if (cursor_line >= MAX_LINEAS) scroll_screen();
            update_cursor(cursor_line, 0);
            return;
        } else if (key == '\b') {
            if (index > 0 && col > 0) {
                index--;
                col--;
                int pos = (row * 80 + col) * 2;
                video_memory[pos] = ' ';
                video_memory[pos + 1] = TEXT_WHITE;
                update_cursor(row, col);
            }
        } else if (index < max_length - 1) {
            buffer[index++] = key;
            int pos = (row * 80 + col) * 2;
            video_memory[pos] = key;
            video_memory[pos + 1] = TEXT_WHITE;
            col++;
            update_cursor(row, col);
        }
    }
}

char get_input() {
    char key;

    while (1) {
        if (inb(0x64) & 0x1) {
            unsigned char scancode = inb(0x60);

            if (scancode == 0x2A || scancode == 0x36) { shift = 1; continue; }
            if (scancode == 0xAA || scancode == 0xB6) { shift = 0; continue; }
            if (scancode == 0x3A) { caps_lock = !caps_lock; continue; }
            if (scancode & 0x80) { continue; }

            switch (scancode) {
                case 0x01: return 27;   // ESC
                case 0x4B: return '<';  // Left
                case 0x4D: return '>';  // Right
                case 0x48: return '^';  // Up
                case 0x50: return 'v';  // Down
                case 0x53: return 127;  // Delete
                case 0x3B: return 'F';  // F1
                case 0x3C: return 'G';  // F2
            }

            int is_letter = (scancode >= 0x10 && scancode <= 0x32);
            int use_shifted = shift || (caps_lock && is_letter);

            key = use_shifted ? shifted_scancode_to_ascii[scancode] : scancode_to_ascii[scancode];
            if (key != 0) return key;
        }
    }
}

// ------------------------ UI Screens (Main Menu, About, Calculator) ------------------------

void display_ui() {
    char input_buffer[20];

    clear_screen(); // Reset screen and cursor_line = 0

    print_to_screen("                               officerdownOS\n");
    print_to_screen("-------------------------------------------------------------------------------\n");
    print_to_screen("\n");  // extra vertical space
    print_to_screen("    ---------------          ---------------\n");
    print_to_screen("\n");
    print_to_screen("           ?                      2+2=4\n");
    print_to_screen("\n");
    print_to_screen("    ---------------          ---------------\n");
    print_to_screen("\n");
    print_to_screen("   About (Press 'a')      Calculator (Press 'c')\n");
    print_to_screen("\n");
    print_to_screen("Enter your choice: ");

    get_string(input_buffer, sizeof(input_buffer));

    if (strcmp(input_buffer, "a") == 0 || strcmp(input_buffer, "about") == 0) {
        display_about();
    } else if (strcmp(input_buffer, "c") == 0 || strcmp(input_buffer, "calc") == 0) {
        display_calculator();
    } else {
        print_to_screen("\nThis is not recognized. Try again!\n");
        print_to_screen("Press any key to continue...");
        get_input();
        display_ui(); // Redraw the screen
    }
}






void display_about() {
    char about_input[20];

    clear_screen();
    print_to_screen("                                      About\n");
    print_to_screen("-------------------------------------------------------------------------------\n");
    print_to_screen("  officerdownOS Main Branch v0.1.5\n");
    print_to_screen("  officerdownOS Rocky v1.0.0\n");
    print_to_screen("  Committed 05/27/2025\n");
    print_to_screen("\nPress 'b' to go back to the main menu.");

    update_cursor(cursor_line, 0);
    get_string(about_input, sizeof(about_input));

    if (strcmp(about_input, "b") == 0) {
        display_ui();
    } else {
        print_to_screen("\nInvalid input. Press 'b' to go back.");
    }
}

void display_calculator() {
    char input[10];
    char num1_str[10], num2_str[10];
    int num1, num2, result = 0;

main_menu:
    clear_screen();
    print_to_screen("                                     Office Calculator\n");
    print_to_screen("------------------------------------------------------------------------------------------------\n\n");
    print_to_screen("Calculator version 1.0\n\n");
    print_to_screen("---------------------------\n\n");
    print_to_screen("/////////////////////////////////////\n");

    char prev[32];
    itoa(previous_result, prev, 10);
    print_to_screen("Your previous Calculated number was ");
    print_to_screen(prev);
    print_to_screen("\n/////////////////////////////////////\n\n");

    print_to_screen("Enter the specified alphabet to continue..\n\n");
    print_to_screen("a) Addition\n\n");
    print_to_screen("b) Subtraction\n\n");
    print_to_screen("c) Division\n\n");
    print_to_screen("d) Multiplication\n\n");
    print_to_screen("e) Square, Cube or any power (by Prof. Pickle)\n\n");
    print_to_screen("exit) Exits\n\n");

    print_to_screen("Choice: ");
    get_string(input, sizeof(input));

    if (strcmp(input, "exit") == 0) return;

    if (strcmp(input, "a") == 0 || strcmp(input, "A") == 0) {
        print_to_screen("\nADDITION\n\nnum1: ");
        get_string(num1_str, sizeof(num1_str));
        print_to_screen("       +\nnum2: ");
        get_string(num2_str, sizeof(num2_str));
        num1 = atoi(num1_str);
        num2 = atoi(num2_str);
        result = num1 + num2;
    } else if (strcmp(input, "b") == 0 || strcmp(input, "B") == 0) {
        print_to_screen("\nSUBTRACTION\n\nnum1: ");
        get_string(num1_str, sizeof(num1_str));
        print_to_screen("       -\nnum2: ");
        get_string(num2_str, sizeof(num2_str));
        num1 = atoi(num1_str);
        num2 = atoi(num2_str);
        result = num1 - num2;
    } else if (strcmp(input, "c") == 0 || strcmp(input, "C") == 0) {
        print_to_screen("\nDIVISION\n\nnum1: ");
        get_string(num1_str, sizeof(num1_str));
        print_to_screen("       /\nnum2: ");
        get_string(num2_str, sizeof(num2_str));
        num1 = atoi(num1_str);
        num2 = atoi(num2_str);
        if (num2 == 0) {
            print_to_screen("Error: Division by zero!\n");
            print_to_screen("Press any key to continue...");
            get_input();
            goto main_menu;
        }
        result = num1 / num2;
    } else if (strcmp(input, "d") == 0 || strcmp(input, "D") == 0) {
        print_to_screen("\nMULTIPLICATION\n\nnum1: ");
        get_string(num1_str, sizeof(num1_str));
        print_to_screen("       *\nnum2: ");
        get_string(num2_str, sizeof(num2_str));
        num1 = atoi(num1_str);
        num2 = atoi(num2_str);
        result = num1 * num2;
    } else if (strcmp(input, "e") == 0 || strcmp(input, "E") == 0) {
        print_to_screen("\nSquare, Cube or any power (by Prof. Pickle)\n\nSelect the number: ");
        get_string(num1_str, sizeof(num1_str));
        print_to_screen("Select the power: ");
        get_string(num2_str, sizeof(num2_str));
        num1 = atoi(num1_str);
        num2 = atoi(num2_str);
        if (num2 < 0 || num2 > 12) {
            print_to_screen("Error: Power too large or invalid.\n");
            print_to_screen("Press any key to continue...");
            get_input();
            goto main_menu;
        }
        result = 1;
        for (int i = 0; i < num2; i++) {
            result *= num1;
        }
    } else {
        print_to_screen("\nInvalid value. Press any key to try again...\n");
        get_input();
        goto main_menu;
    }

    previous_result = result;
    char result_str[16];
    itoa(result, result_str, 10);
    print_to_screen("\n------------\n");
    print_to_screen(result_str);
    print_to_screen("\n\nPress any key to return to menu...");
    get_input();
    goto main_menu;
}

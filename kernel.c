#include "kernel.h"
#include "screen.h"

void display_ui(void);
void display_about(void);
void display_calculator(void);

// Global state
int shift = 0;
int caps_lock = 0;
int previous_result = 0;

#define TXT_BLANCO 0x07
#define KEYBOARD_PORT 0x60
#define MAX_LINEAS 25

char *memoria_video = (char *)0xb8000;
unsigned int linea = 0;

// ------------------------ Keyboard mappings ------------------------
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

// ------------------------ Utility Functions ------------------------

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

// ------------------------ Core OS Functions ------------------------

void kernel_main() {
    limpiar_pantalla();
    while (1) {
        display_ui();
    }
}

void limpiar_pantalla() {
    for (unsigned int i = 0; i < (80 * 25 * 2); i += 2) {
        memoria_video[i] = ' ';
        memoria_video[i + 1] = TXT_BLANCO;
    }
    linea = 0;
}

void scroll_pantalla() {
    for (int row = 1; row < MAX_LINEAS; row++) {
        for (int col = 0; col < 80; col++) {
            int from = (row * 80 + col) * 2;
            int to = ((row - 1) * 80 + col) * 2;
            memoria_video[to] = memoria_video[from];
            memoria_video[to + 1] = memoria_video[from + 1];
        }
    }

    // Clear last line
    for (int col = 0; col < 80; col++) {
        int pos = ((MAX_LINEAS - 1) * 80 + col) * 2;
        memoria_video[pos] = ' ';
        memoria_video[pos + 1] = TXT_BLANCO;
    }

    linea = MAX_LINEAS - 1;
}

unsigned int imprimir_pantalla(char *string) {
    int col = 0; // 🔧 Reset every time

    while (*string != 0) {
        if (*string == '\n') {
            linea++;
            col = 0;
            if (linea >= MAX_LINEAS) {
                scroll_pantalla();
            }
        } else {
            int pos = (linea * 80 + col) * 2;
            memoria_video[pos] = *string;
            memoria_video[pos + 1] = TXT_BLANCO;
            col++;

            if (col >= 80) {
                linea++;
                col = 0;
                if (linea >= MAX_LINEAS) {
                    scroll_pantalla();
                }
            }
        }

        string++;
    }

    actualizar_cursor(linea, col);
    return 1;
}





void actualizar_cursor(int row, int col) {
    unsigned short position = (row * 80) + col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

void get_string(char *buffer, int max_length) {
    int index = 0;
    static unsigned int col = 0;

    if (linea >= MAX_LINEAS) scroll_pantalla();
    int row = linea;
    col = 0;

    actualizar_cursor(row, col);

    while (1) {
        char key = get_input();

        if (key == '\n') {
            buffer[index] = '\0';
            linea++;
            if (linea >= MAX_LINEAS) scroll_pantalla();
            actualizar_cursor(linea, 0);
            return;
        } else if (key == '\b') {
            if (index > 0 && col > 0) {
                index--;
                col--;
                int pos = (row * 80 + col) * 2;
                memoria_video[pos] = ' ';
                memoria_video[pos + 1] = TXT_BLANCO;
                actualizar_cursor(row, col);
            }
        } else if (index < max_length - 1) {
            buffer[index++] = key;
            int pos = (row * 80 + col) * 2;
            memoria_video[pos] = key;
            memoria_video[pos + 1] = TXT_BLANCO;
            col++;
            actualizar_cursor(row, col);
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

// ------------------------ UI Screens ------------------------

void display_ui() {
    char input_buffer[20];

    limpiar_pantalla(); // Reset screen and linea = 0

    imprimir_pantalla("                               officerdownOS\n");
    imprimir_pantalla("-------------------------------------------------------------------------------\n");
    imprimir_pantalla("\n");  // extra vertical space
    imprimir_pantalla("    ---------------          ---------------\n");
    imprimir_pantalla("\n");
    imprimir_pantalla("           ?                      2+2=4\n");
    imprimir_pantalla("\n");
    imprimir_pantalla("    ---------------          ---------------\n");
    imprimir_pantalla("\n");
    imprimir_pantalla("   About (Press 'a')      Calculator (Press 'c')\n");
    imprimir_pantalla("\n");
    imprimir_pantalla("Enter your choice: ");

    get_string(input_buffer, sizeof(input_buffer));

    if (strcmp(input_buffer, "a") == 0 || strcmp(input_buffer, "about") == 0) {
        display_about();
    } else if (strcmp(input_buffer, "c") == 0 || strcmp(input_buffer, "calc") == 0) {
        display_calculator();
    } else {
        imprimir_pantalla("\nThis is not recognized. Try again!\n");
        imprimir_pantalla("Press any key to continue...");
        get_input();
        display_ui(); // Redraw the screen
    }
}






void display_about() {
    char about_input[20];

    limpiar_pantalla();
    imprimir_pantalla("                                      About\n");
    imprimir_pantalla("-------------------------------------------------------------------------------\n");
    imprimir_pantalla("  officerdownOS Main Branch v0.1.5\n");
    imprimir_pantalla("  officerdownOS Rocky v1.0.0\n");
    imprimir_pantalla("  Committed 05/27/2025\n");
    imprimir_pantalla("\nPress 'b' to go back to the main menu.");

    actualizar_cursor(linea, 0);
    get_string(about_input, sizeof(about_input));

    if (strcmp(about_input, "b") == 0) {
        display_ui();
    } else {
        imprimir_pantalla("\nInvalid input. Press 'b' to go back.");
    }
}

void display_calculator() {
    char input[10];
    char num1_str[10], num2_str[10];
    int num1, num2, result = 0;

main_menu:
    limpiar_pantalla();
    imprimir_pantalla("                                     Office Calculator\n");
    imprimir_pantalla("------------------------------------------------------------------------------------------------\n\n");
    imprimir_pantalla("Calculator version 1.0\n\n");
    imprimir_pantalla("---------------------------\n\n");
    imprimir_pantalla("/////////////////////////////////////\n");

    char prev[32];
    itoa(previous_result, prev, 10);
    imprimir_pantalla("Your previous Calculated number was ");
    imprimir_pantalla(prev);
    imprimir_pantalla("\n/////////////////////////////////////\n\n");

    imprimir_pantalla("Enter the specified alphabet to continue..\n\n");
    imprimir_pantalla("a) Addition\n\n");
    imprimir_pantalla("b) Subtraction\n\n");
    imprimir_pantalla("c) Division\n\n");
    imprimir_pantalla("d) Multiplication\n\n");
    imprimir_pantalla("e) Square, Cube or any power (by Prof. Pickle)\n\n");
    imprimir_pantalla("exit) Exits\n\n");

    imprimir_pantalla("Choice: ");
    get_string(input, sizeof(input));

    if (strcmp(input, "exit") == 0) return;

    if (strcmp(input, "a") == 0 || strcmp(input, "A") == 0) {
        imprimir_pantalla("\nADDITION\n\nnum1: ");
        get_string(num1_str, sizeof(num1_str));
        imprimir_pantalla("       +\nnum2: ");
        get_string(num2_str, sizeof(num2_str));
        num1 = atoi(num1_str);
        num2 = atoi(num2_str);
        result = num1 + num2;
    } else if (strcmp(input, "b") == 0 || strcmp(input, "B") == 0) {
        imprimir_pantalla("\nSUBTRACTION\n\nnum1: ");
        get_string(num1_str, sizeof(num1_str));
        imprimir_pantalla("       -\nnum2: ");
        get_string(num2_str, sizeof(num2_str));
        num1 = atoi(num1_str);
        num2 = atoi(num2_str);
        result = num1 - num2;
    } else if (strcmp(input, "c") == 0 || strcmp(input, "C") == 0) {
        imprimir_pantalla("\nDIVISION\n\nnum1: ");
        get_string(num1_str, sizeof(num1_str));
        imprimir_pantalla("       /\nnum2: ");
        get_string(num2_str, sizeof(num2_str));
        num1 = atoi(num1_str);
        num2 = atoi(num2_str);
        if (num2 == 0) {
            imprimir_pantalla("Error: Division by zero!\n");
            imprimir_pantalla("Press any key to continue...");
            get_input();
            goto main_menu;
        }
        result = num1 / num2;
    } else if (strcmp(input, "d") == 0 || strcmp(input, "D") == 0) {
        imprimir_pantalla("\nMULTIPLICATION\n\nnum1: ");
        get_string(num1_str, sizeof(num1_str));
        imprimir_pantalla("       *\nnum2: ");
        get_string(num2_str, sizeof(num2_str));
        num1 = atoi(num1_str);
        num2 = atoi(num2_str);
        result = num1 * num2;
    } else if (strcmp(input, "e") == 0 || strcmp(input, "E") == 0) {
        imprimir_pantalla("\nSquare, Cube or any power (by Prof. Pickle)\n\nSelect the number: ");
        get_string(num1_str, sizeof(num1_str));
        imprimir_pantalla("Select the power: ");
        get_string(num2_str, sizeof(num2_str));
        num1 = atoi(num1_str);
        num2 = atoi(num2_str);
        if (num2 < 0 || num2 > 12) {
            imprimir_pantalla("Error: Power too large or invalid.\n");
            imprimir_pantalla("Press any key to continue...");
            get_input();
            goto main_menu;
        }
        result = 1;
        for (int i = 0; i < num2; i++) {
            result *= num1;
        }
    } else {
        imprimir_pantalla("\nInvalid value. Press any key to try again...\n");
        get_input();
        goto main_menu;
    }

    previous_result = result;
    char result_str[16];
    itoa(result, result_str, 10);
    imprimir_pantalla("\n------------\n");
    imprimir_pantalla(result_str);
    imprimir_pantalla("\n\nPress any key to return to menu...");
    get_input();
    goto main_menu;
}

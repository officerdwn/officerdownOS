#include "kernel.h"
#include "screen.h"

// Forward declarations for functions
void display_ui();
void display_about();
char get_input();
void actualizar_cursor(int row, int col);
int strcmp(const char *s1, const char *s2);

// Scancode to ASCII mapping (only for lowercase letters and numbers)
char scancode_to_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', // Backspace
    '\t', // Tab
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', // Enter
    0, // Control
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,  // Left Shift
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,       // Right Shift
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


// ✅ Function to update the cursor position
void actualizar_cursor(int row, int col) {
    unsigned short position = (row * 80) + col;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

// ✅ String comparison function
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

#define TXT_BLANCO 0x07 // White on black
#define KEYBOARD_PORT 0x60 // I/O port for the keyboard

char *memoria_video = (char *)0xb8000;
unsigned int linea = 0;

void kernel_main() {
    limpiar_pantalla(); // Clear the screen

    while (1) {
        display_ui(); // Display the UI and handle user input
    }
}

void limpiar_pantalla() {
    unsigned int i = 0;
    while (i < (80 * 25 * 2)) {
        memoria_video[i] = ' ';
        i++;
        memoria_video[i] = TXT_BLANCO;
        i++;
    }
    linea = 0; // Reset line counter
}

// ✅ Improved printing function
unsigned int imprimir_pantalla(char *string) {
    unsigned int i = (linea * 80 * 2);

    while (*string != 0) {
        if (*string == '\n') {
            linea++;
            i = (linea * 80 * 2);
            string++;
        } else {
            memoria_video[i] = *string;
            i++;
            memoria_video[i] = TXT_BLANCO;
            i++;
            string++;
        }
    }
    linea++;
    actualizar_cursor(linea, 0); // ✅ Updates cursor after printing
    return 1;
}

// ✅ Improved input handling
void get_string(char *buffer, int max_length) {
    int index = 0;

    while (1) {
        char key = get_input(); // Read a single key from the keyboard

        if (key == '\n') {  // Enter key
            buffer[index] = '\0';  // Null-terminate the string
            imprimir_pantalla("\n");
            return;
        } else if (key == '\b') { // Handle backspace
            if (index > 0) {
                index--;
                imprimir_pantalla("\b \b"); // Erase last character on screen
            }
        } else if (index < max_length - 1) { // Normal character
            buffer[index++] = key;
            char str[2] = {key, '\0'};
            imprimir_pantalla(str);
        }
    }
}

// ✅ Improved UI
void display_ui() {
    char input_buffer[20];

    limpiar_pantalla(); // Clear the screen

    imprimir_pantalla("                              officerdownOS\n");
    imprimir_pantalla("-------------------------------------------------------------------------------\n");
    imprimir_pantalla("    ---------------           ---------------\n");
    imprimir_pantalla("            ?                      2+2=4\n");
    imprimir_pantalla("    ---------------           ---------------\n");
    imprimir_pantalla("      About (Press 'a')     Calculator (Press 'c')\n");
    imprimir_pantalla("\nEnter your choice: ");

    actualizar_cursor(10, 18); // ✅ Correct cursor position

    get_string(input_buffer, sizeof(input_buffer));

    if (strcmp(input_buffer, "a") == 0 || strcmp(input_buffer, "about") == 0) {
        display_about();
        return;
        } else if (strcmp(input_buffer, "c") == 0 || strcmp(input_buffer, "calc") == 0) {
        display_calculator();
        return;
    } else {
        imprimir_pantalla("\nThis is not recognized. Try again!\n");
    }

    imprimir_pantalla("\nPress any key to continue...");
    get_input();
}

// ✅ Improved "About" screen
void display_about() {
    char about_input[20];

    limpiar_pantalla();
    imprimir_pantalla("                                      About\n");
    imprimir_pantalla("-------------------------------------------------------------------------------\n");
    imprimir_pantalla("  officerdownOS v0.1.3\n");
    imprimir_pantalla("  Committed 02/20/2025\n");
    imprimir_pantalla("\nPress 'b' to go back to the main menu.");

    actualizar_cursor(10, 18); // ✅ Correct cursor positioning

    get_string(about_input, sizeof(about_input));

    if (strcmp(about_input, "b") == 0) {
        display_ui();
        return;
    } else {
        imprimir_pantalla("\nInvalid input. Press 'b' to go back.");
    }
}

void display_calculator() {
    char num1_str[10], num2_str[10], op_str[2]; // Store operator as a string
    int num1, num2, result = 0;

    limpiar_pantalla();
    imprimir_pantalla("                                    Calculator\n");
    imprimir_pantalla("-------------------------------------------------------------------------------\n");

    // Get first number
    imprimir_pantalla("Enter first number: ");
    get_string(num1_str, sizeof(num1_str));

    // Get operator
    imprimir_pantalla("Enter an operator (+, -, *, /): ");
    get_string(op_str, sizeof(op_str)); // Read the operator as a string
    char op = op_str[0]; // Extract the operator character
    imprimir_pantalla("\n");

    // Get second number
    imprimir_pantalla("Enter second number: ");
    get_string(num2_str, sizeof(num2_str));

    // Convert input strings to numbers
    num1 = atoi(num1_str);
    num2 = atoi(num2_str);

    // Perform calculation
    switch (op) {
        case '+':
            result = num1 + num2;
            break;
        case '-':
            result = num1 - num2;
            break;
        case '*':
        case 'x': // Allow both '*' and 'x' for multiplication
            result = num1 * num2;
            break;
        case '/':
            if (num2 != 0) {
                result = num1 / num2;
            } else {
                imprimir_pantalla("Error: Division by zero!\n");
                imprimir_pantalla("\nPress any key to return to the main menu...");
                get_input();
                display_ui();
                return;
            }
            break;
        default:
            imprimir_pantalla("Invalid operator! Use +, -, *, /.\n");
            imprimir_pantalla("\nPress any key to return to the main menu...");
            get_input();
            display_ui();
            return;
    }

    // Convert result to string and display it
    char result_str[12];
    itoa(result, result_str, 10);
    imprimir_pantalla("Result: ");
    imprimir_pantalla(result_str);
    imprimir_pantalla("\n\nPress any key to return to the main menu...");
    
    get_input(); // ✅ Pause for input before returning
    display_ui(); // ✅ Return to main menu after user presses a key
}



int atoi(const char *str) {
    int result = 0, sign = 1;
    if (*str == '-') { // Handle negative numbers
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
    int i = 0;
    int is_negative = 0;

    // Edge case: If num is 0, just return "0"
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Handle negative numbers for base 10
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }

    // Process individual digits
    while (num > 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'A' : rem + '0'; // Use uppercase for hex
        num /= base;
    }

    // Append negative sign for negative numbers
    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0'; // Null-terminate the string

    // Reverse the string
    int start = 0, end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}


char get_input() {
    char key;

    while (1) {
        if (inb(0x64) & 0x1) { // Check if a key is available
            unsigned char scancode = inb(0x60); // Read scancode

            // Ignore key releases (break codes, scancode + 0x80)
            if (scancode & 0x80) {
                continue;
            }

            // Convert scancode to ASCII
            key = scancode_to_ascii[scancode];

            // Return valid key
            if (key != 0) {
                return key;
            }
        }
    }
}



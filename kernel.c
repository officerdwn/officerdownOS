#include "kernel.h"
#include "screen.h"

// Forward declarations for functions
void display_ui();
void display_about();
char get_input();

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

unsigned int imprimir_pantalla(char *string) {
    unsigned int i = (linea * 80 * 2);

    while (*string != 0) {
        if (*string == '\n') {
            linea++;
            i = (linea * 80 * 2);
            string++;
        } else {
            memoria_video[i] = *string;
            string++;
            i++;
            memoria_video[i] = TXT_BLANCO;
            i++;
        }
    }
    linea++;
    return 1;
}

void display_ui() {
    char desktopOptions;

    limpiar_pantalla(); // Clear the screen

    imprimir_pantalla("                                 officerdownOS\n");
    imprimir_pantalla("----------------------------------------------------------------------------------\n");
    imprimir_pantalla("            ---------------                  \n");
    imprimir_pantalla("\n");
    imprimir_pantalla("\n");
    imprimir_pantalla("                   ?                             \n");
    imprimir_pantalla("\n");
    imprimir_pantalla("           ----------------                 \n");
    imprimir_pantalla("\n");
    imprimir_pantalla("                About (Press 'a')            \n");
    imprimir_pantalla("\n");
    imprimir_pantalla("Enter your choice: ");

    desktopOptions = get_input(); // Get user input

    switch (desktopOptions) {
        case 'a':
            display_about(); // Call the About screen function
            return;
        default:
            imprimir_pantalla("\nThis is not recognized. Try again!\n");
            break;
    }

    imprimir_pantalla("\nPress any key to continue...");
    get_input(); // Wait for user to press a key
}

void display_about() {
    char aboutChoice;

    limpiar_pantalla(); // Clear the screen
    imprimir_pantalla("                                    About\n");
    imprimir_pantalla("----------------------------------------------------------------------------------\n");
    imprimir_pantalla("  officerdownOS v0.1.1\n");
    imprimir_pantalla("  Compiled 01/25/2025\n");
    imprimir_pantalla("\nPress 'b' to go back to the main menu.");

    while (1) {
        aboutChoice = get_input(); // Get user input

        if (aboutChoice == 'b') {
            display_ui(); // Return to the main menu
            return;
        } else {
            imprimir_pantalla("\nInvalid input. Press 'b' to go back.");
        }
    }
}

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

char get_input() {
    char key;
    
    // Wait for a valid key press
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


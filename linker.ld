/* Specify the entry point */
ENTRY(start)

/* Define multiboot header constants */
MULTIBOOT_PAGE_ALIGN     = 1 << 0;  /* Align modules on page boundaries */
MULTIBOOT_MEMORY_INFO    = 1 << 1;  /* Request memory map from GRUB */
MULTIBOOT_HEADER_FLAGS   = MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO;

SECTIONS {
    /* The kernel starts at 1MB */
    . = 0x00100000;

    /* First section must contain the multiboot header */
    .text ALIGN(4K) : {
        /* Ensure multiboot header is at the beginning */
        *(.text.boot)
        *(.text)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data ALIGN(4K) : {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss ALIGN(4K) : {
        sbss = .;
        *(COMMON)
        *(.bss)
        ebss = .;
    }

    /* Discard debug info */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}

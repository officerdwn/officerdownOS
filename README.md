# officerdownOS

A minimal hobby operating system.

## Filesystem

A minimal FAT12 filesystem driver lets you read files from the first disk.
From the boot screen, press `f` to explore the filesystem.

Available operations:

- `l` – list files in the root directory
- `v` – view file contents
- `b` – return to the main menu

The filesystem is read-only. To experiment, boot with a FAT-formatted disk image
containing the files you want to inspect.


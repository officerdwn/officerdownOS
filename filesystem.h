#ifndef FILESYSTEM_H
#define FILESYSTEM_H

void fs_init();
void fs_list();
int fs_read_file(const char *name, char *buffer);
unsigned int fs_free_space();
int fs_write_file(const char *name, const char *content);

#endif


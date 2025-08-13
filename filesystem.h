#ifndef FILESYSTEM_H
#define FILESYSTEM_H

void fs_init();
void fs_list();
int fs_read_file(const char *name, char *buffer);

#endif


#ifndef DISK_H
#define DISK_H

void read_sector(unsigned int lba, unsigned char *buffer);
void write_sector(unsigned int lba, const unsigned char *buffer);

#endif


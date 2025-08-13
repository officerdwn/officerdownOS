#include "filesystem.h"
#include "disk.h"
#include "kernel.h"

#define SECTOR_SIZE 512

typedef struct __attribute__((packed)) {
    unsigned char name[11];
    unsigned char attr;
    unsigned char reserved[10];
    unsigned short time;
    unsigned short date;
    unsigned short first_cluster;
    unsigned int size;
} DirectoryEntry;

static unsigned short bytes_per_sector;
static unsigned char sectors_per_cluster;
static unsigned short reserved_sectors;
static unsigned char num_fats;
static unsigned short root_entries;
static unsigned short fat_size;
static unsigned int fat_start;
static unsigned int root_start;
static unsigned int data_start;

static unsigned char fat[4608];

static unsigned int str_len(const char *s) {
    unsigned int i = 0;
    while (s[i]) i++;
    return i;
}

static void format_name(const unsigned char *raw, char *out) {
    int i = 0;
    for (int j = 0; j < 8 && raw[j] != ' '; j++) {
        out[i++] = raw[j];
    }
    if (raw[8] != ' ') {
        out[i++] = '.';
        for (int j = 8; j < 11 && raw[j] != ' '; j++) {
            out[i++] = raw[j];
        }
    }
    out[i] = '\0';
}

static void format_search_name(const char *in, unsigned char *out) {
    int i = 0;
    for (; i < 11; i++) out[i] = ' ';
    int j = 0;
    while (in[j] && in[j] != '.' && j < 8) {
        char c = in[j];
        if (c >= 'a' && c <= 'z') c -= 32;
        out[j] = c;
        j++;
    }
    if (in[j] == '.') {
        j++;
        int k = 0;
        while (in[j] && k < 3) {
            char c = in[j];
            if (c >= 'a' && c <= 'z') c -= 32;
            out[8 + k] = c;
            j++; k++;
        }
    }
}

static int name_equals(const unsigned char *a, const unsigned char *b) {
    for (int i = 0; i < 11; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

static unsigned short fat_next_cluster(unsigned short cluster) {
    unsigned int index = cluster + cluster / 2;
    unsigned short value = fat[index] | (fat[index + 1] << 8);
    if (cluster & 1) {
        value >>= 4;
    } else {
        value &= 0x0FFF;
    }
    return value;
}

static unsigned short fat_get(unsigned short cluster) {
    unsigned int index = cluster + cluster / 2;
    unsigned short value = fat[index] | (fat[index + 1] << 8);
    if (cluster & 1) {
        value >>= 4;
    } else {
        value &= 0x0FFF;
    }
    return value;
}

static void fat_set(unsigned short cluster, unsigned short value) {
    unsigned int index = cluster + cluster / 2;
    if (cluster & 1) {
        fat[index] = (fat[index] & 0x0F) | ((value & 0x0F) << 4);
        fat[index + 1] = (value >> 4) & 0xFF;
    } else {
        fat[index] = value & 0xFF;
        fat[index + 1] = (fat[index + 1] & 0xF0) | ((value >> 8) & 0x0F);
    }
}

void fs_init() {
    unsigned char sector[SECTOR_SIZE];
    read_sector(0, sector);

    bytes_per_sector = sector[11] | (sector[12] << 8);
    sectors_per_cluster = sector[13];
    reserved_sectors = sector[14] | (sector[15] << 8);
    num_fats = sector[16];
    root_entries = sector[17] | (sector[18] << 8);
    fat_size = sector[22] | (sector[23] << 8);

    fat_start = reserved_sectors;
    root_start = fat_start + num_fats * fat_size;
    unsigned int root_sectors = ((root_entries * 32) + (bytes_per_sector - 1)) / bytes_per_sector;
    data_start = root_start + root_sectors;

    for (int i = 0; i < fat_size; i++) {
        read_sector(fat_start + i, fat + i * SECTOR_SIZE);
    }
}

void fs_list() {
    unsigned int root_sectors = ((root_entries * 32) + (bytes_per_sector - 1)) / bytes_per_sector;
    unsigned char sector[SECTOR_SIZE];

    for (unsigned int i = 0; i < root_sectors; i++) {
        read_sector(root_start + i, sector);
        DirectoryEntry *entries = (DirectoryEntry *)sector;
        for (int j = 0; j < bytes_per_sector / 32; j++) {
            if (entries[j].name[0] == 0x00) return;
            if (entries[j].name[0] == 0xE5) continue;
            if (entries[j].attr == 0x0F) continue;
            char name[13];
            format_name(entries[j].name, name);
            print_to_screen(name);
            print_to_screen("\n");
        }
    }
}

static int find_entry(const char *name, DirectoryEntry *out) {
    unsigned char search[11];
    format_search_name(name, search);

    unsigned int root_sectors = ((root_entries * 32) + (bytes_per_sector - 1)) / bytes_per_sector;
    unsigned char sector[SECTOR_SIZE];

    for (unsigned int i = 0; i < root_sectors; i++) {
        read_sector(root_start + i, sector);
        DirectoryEntry *entries = (DirectoryEntry *)sector;
        for (int j = 0; j < bytes_per_sector / 32; j++) {
            if (entries[j].name[0] == 0x00) return 0;
            if (entries[j].name[0] == 0xE5) continue;
            if (entries[j].attr == 0x0F) continue;
            if (name_equals(entries[j].name, search)) {
                *out = entries[j];
                return 1;
            }
        }
    }
    return 0;
}

int fs_read_file(const char *name, char *buffer) {
    DirectoryEntry entry;
    if (!find_entry(name, &entry)) {
        return -1;
    }

    unsigned int cluster = entry.first_cluster;
    unsigned int bytes_read = 0;
    unsigned char sector[SECTOR_SIZE];

    while (cluster < 0xFF8) {
        unsigned int lba = data_start + (cluster - 2) * sectors_per_cluster;
        for (int s = 0; s < sectors_per_cluster; s++) {
            read_sector(lba + s, sector);
            for (int b = 0; b < bytes_per_sector && bytes_read < entry.size; b++) {
                buffer[bytes_read++] = sector[b];
            }
        }
        cluster = fat_next_cluster(cluster);
    }

    buffer[bytes_read] = '\0';
    return (int)bytes_read;
}

unsigned int fs_free_space() {
    unsigned int total_clusters = (fat_size * SECTOR_SIZE * 2) / 3;
    unsigned int free_clusters = 0;
    for (unsigned int c = 2; c < total_clusters; c++) {
        if (fat_get(c) == 0x000) free_clusters++;
    }
    return free_clusters * sectors_per_cluster * bytes_per_sector;
}

int fs_write_file(const char *name, const char *content) {
    unsigned int size = str_len(content);
    unsigned int cluster_bytes = bytes_per_sector * sectors_per_cluster;
    unsigned int needed = (size + cluster_bytes - 1) / cluster_bytes;
    unsigned int total_clusters = (fat_size * SECTOR_SIZE * 2) / 3;

    unsigned short clusters[32];
    unsigned int found = 0;
    for (unsigned int c = 2; c < total_clusters && found < needed; c++) {
        if (fat_get(c) == 0x000) {
            clusters[found++] = c;
        }
    }
    if (found < needed) return -1;

    unsigned int written = 0;
    unsigned char sector[SECTOR_SIZE];
    for (unsigned int i = 0; i < needed; i++) {
        unsigned int lba = data_start + (clusters[i] - 2) * sectors_per_cluster;
        for (int s = 0; s < sectors_per_cluster; s++) {
            for (int b = 0; b < bytes_per_sector; b++) {
                if (written < size) {
                    sector[b] = content[written++];
                } else {
                    sector[b] = 0;
                }
            }
            write_sector(lba + s, sector);
        }
        if (i == needed - 1) fat_set(clusters[i], 0xFFF);
        else fat_set(clusters[i], clusters[i + 1]);
    }

    for (unsigned int i = 0; i < fat_size; i++) {
        write_sector(fat_start + i, fat + i * SECTOR_SIZE);
        write_sector(fat_start + i + fat_size, fat + i * SECTOR_SIZE);
    }

    unsigned int root_sectors = ((root_entries * 32) + (bytes_per_sector - 1)) / bytes_per_sector;
    unsigned char dir_sector[SECTOR_SIZE];
    for (unsigned int i = 0; i < root_sectors; i++) {
        read_sector(root_start + i, dir_sector);
        DirectoryEntry *entries = (DirectoryEntry *)dir_sector;
        for (int j = 0; j < bytes_per_sector / 32; j++) {
            if (entries[j].name[0] == 0x00 || entries[j].name[0] == 0xE5) {
                format_search_name(name, entries[j].name);
                entries[j].attr = 0x20;
                entries[j].first_cluster = clusters[0];
                entries[j].size = size;
                write_sector(root_start + i, dir_sector);
                return 0;
            }
        }
    }
    return -1;
}


#ifndef SHAREDFILE_SHAREDFILE_H
#define SHAREDFILE_SHAREDFILE_H

extern int difficulty;  // Declare the variable as extern

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <zlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#define CONFIG_FILE "/mnt/mta/mtacoin.conf"

typedef struct BLOCK_T {
    int height;
    int timestamp;
    unsigned int hash;
    unsigned int prev_hash;
    int difficulty;
    int nonce;
    int relayed_by;
} BLOCK_T;

unsigned int calculate_crc32(BLOCK_T block);

bool verify_difficulty(unsigned int hash);

int get_difficulty();

void create_log_file();
#endif //SHAREDFILE_SHAREDFILE_H

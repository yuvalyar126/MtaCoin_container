#include "SharedFile.h"


unsigned int calculate_crc32(BLOCK_T block) {
    unsigned int crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (const unsigned char *) &block.height, sizeof(block.height));
    crc = crc32(crc, (const unsigned char *) &block.timestamp, sizeof(block.timestamp));
    crc = crc32(crc, (const unsigned char *) &block.prev_hash, sizeof(block.prev_hash));
    crc = crc32(crc, (const unsigned char *) &block.nonce, sizeof(block.nonce));
    crc = crc32(crc, (const unsigned char *) &block.relayed_by, sizeof(block.relayed_by));
    return crc;
}


bool verify_difficulty(unsigned int hash) {
    return (hash >> (32 - difficulty)) == 0;
}

int get_difficulty() {
    int difficulty;
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        perror("Unable to open file");
        exit(-1); // Return an error code if the file cannot be opened
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Find the position of "DIFFICULTY="
        char *pos = strstr(line, "DIFFICULTY=");
        if (pos != NULL) {
            // Extract the substring containing the number
            if (sscanf(pos + 11, "%d", &difficulty) == 1) { // 13 is the length of "DIFFICULTY="
                fclose(file);
            }
        }
    }
    fclose(file);
    return difficulty;
}

void create_log_file() {
    const char *log_file_path = "/var/log/mtacoin.log";
    
    // Open the log file, create it if it doesn't exist, with read/write permissions
    int log_fd = open(log_file_path, O_WRONLY | O_CREAT | O_APPEND, 0777);
    if (log_fd == -1) {
        perror("Failed to create log file");
        exit(EXIT_FAILURE);
    }
    
    
       // Redirect stdout to the log file
    if (dup2(log_fd, STDOUT_FILENO) == -1) {
        perror("Failed to redirect stdout");
        close(log_fd);
        exit(EXIT_FAILURE);
    }

    // Close the file descriptor as it's no longer needed
    close(log_fd);
}



#include "../SharedFile/SharedFile.h"

#define MINER_PIPES_PATH "/mnt/mta/miner_pipe_"
#define MAX_PIPE_NAME_LENGTH  256
#define FILE_PATH "/mnt/mta/miner_count.txt"
#define SERVER_PIPE_PATH "/mnt/mta/server_pipe"


int difficulty;

int increment_miner_count() {
    int fd;
    FILE *file;
    int current_count;
    int new_count;

    // Ensure the directory exists
//    system("mkdir -p /mnt/miners");

    // Open the file with read/write permissions, create it if it doesn't exist
    fd = open(FILE_PATH, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Lock the file to prevent concurrent access
    if (lockf(fd, F_LOCK, 0) == -1) {
        perror("Error locking file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Open the file stream
    file = fdopen(fd, "r+");
    if (!file) {
        perror("Error opening file stream");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Read the current count
    if (fscanf(file, "%d", &current_count) != 1) {
        // If file is empty or unreadable, initialize count to 0
        current_count = 0;
    }

    // Increment the count
    new_count = current_count + 1;

    // Move the file pointer to the beginning and truncate the file
    fseek(file, 0, SEEK_SET);
    ftruncate(fd, 0);

    // Write the new count to the file
    fprintf(file, "%d\n", new_count);

    // Flush and close the file stream
    fflush(file);
    fclose(file);

    // Unlock and close the file descriptor
    lockf(fd, F_ULOCK, 0);
    close(fd);

    // Print the new count
    printf("New miner count: %d\n", new_count);
    return new_count;
}


void miner_function() {
    int miner_id = increment_miner_count();
    char miner_pipe_name[256];
    sprintf(miner_pipe_name, "%s%d", MINER_PIPES_PATH, miner_id);


    mkfifo(miner_pipe_name, 0666);


    int server_pipe_fd = open(SERVER_PIPE_PATH, O_WRONLY);
    if (server_pipe_fd < 0) {
        perror("server_pipe_fd");
        exit(1);
    }
    if (write(server_pipe_fd, miner_pipe_name, strlen(miner_pipe_name) + 1) < 0) {
        perror("writing to server failed");
        exit(-1);
    }
    printf("Miner #%d Sent connection request to server_pipe\n", miner_id);


    BLOCK_T current_block;

    bool new_block_during_mining = false;

    while (1) {
        if (!new_block_during_mining) {
            int miner_pipe_fd = open(miner_pipe_name, O_RDONLY);
            if (miner_pipe_fd < 0) {
                perror("miner_pipe_fd");
                exit(1);
            }
            // Always read the latest block from the pipe before starting new mining work
            if (read(miner_pipe_fd, &current_block, sizeof(BLOCK_T)) == -1) {
                perror("read\n");
                exit(1);
            }

            printf("Miner #%d: Received new block: {height: %d, timestamp: %d, hash: 0x%x, prev_hash: 0x%x, difficulty: %d, nonce: %d, relayed_by: %d}\n",
                   miner_id, current_block.height, current_block.timestamp, current_block.hash, current_block.prev_hash,
                   current_block.difficulty, current_block.nonce, current_block.relayed_by);
            close(miner_pipe_fd);

        }
        new_block_during_mining = false;
        BLOCK_T new_block;
        new_block.height = current_block.height + 1;
        new_block.prev_hash = current_block.hash;
        new_block.difficulty = difficulty;
        new_block.relayed_by = miner_id;
        new_block.nonce = 0;
        new_block.timestamp = time(NULL);
        new_block.hash = calculate_crc32(new_block);


        int miner_pipe_fd = open(miner_pipe_name, O_RDONLY | O_NONBLOCK);
        if (miner_pipe_fd < 0) {
            perror("miner_pipe_fd");
            exit(1);
        }

        while (!verify_difficulty(new_block.hash)) {
            new_block.nonce++;
            new_block.timestamp = time(NULL);
            new_block.hash = calculate_crc32(new_block);
            // printf("Miner #%d: Trying nonce = %d, hash = 0x%x\n", miner_id, new_block.nonce, new_block.hash);

            int bytes_read = read(miner_pipe_fd, &current_block, sizeof(BLOCK_T));
            if (bytes_read > 0) {
                printf("Miner #%d: Received new block during mining: {height: %d, timestamp: %d, hash: 0x%x, prev_hash: 0x%x, difficulty: %d, nonce: %d, relayed_by: %d}\n",
                       miner_id, current_block.height, current_block.timestamp, current_block.hash, current_block.prev_hash,
                       current_block.difficulty, current_block.nonce, current_block.relayed_by);
                new_block_during_mining = true;
                break;
            }
        }

        if (new_block_during_mining == false) {
            if (verify_difficulty(new_block.hash)) {
                int bytesWrite = write(server_pipe_fd, &new_block, sizeof(BLOCK_T));
                if (bytesWrite == -1) {
                    perror("write");
                    exit(1);
                }
                printf("Miner #%d: Mined a new block #%d, with hash 0x%x, difficulty %d\n",
                       miner_id, new_block.height, new_block.hash, new_block.difficulty);
            }
            usleep(500000);
        }
        close(miner_pipe_fd);
    }
}


int main() {
    create_log_file();
    difficulty = get_difficulty();
    miner_function();
    return 0;
}

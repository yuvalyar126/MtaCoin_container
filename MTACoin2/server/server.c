#include "../SharedFile/SharedFile.h"

#define MINER_COUNT "/mnt/mta/miner_count.txt"
#define SERVER_PIPE_PATH "/mnt/mta/server_pipe"
#define MINER_PIPE_PATH "/mnt/mta/miner_pipe_"

int difficulty;
int block_count = 0;
BLOCK_T latest_block;

void server_function() {
    printf("Difficulty set to %d\n", difficulty);
    // Create genesis block
    BLOCK_T genesis_block;
    genesis_block = (BLOCK_T) {0, (int) time(NULL), 0, 0, difficulty, 0, -1};
    genesis_block.hash = calculate_crc32(genesis_block); // Calculate the hash of the genesis block
    latest_block = genesis_block; // Set the genesis block as the latest block
    block_count++; // Add 1 to block count
    printf("Genesis block created: {height: %d, timestamp: %d, hash: 0x%x, prev_hash: 0x%x, difficulty: %d, nonce: %d, relayed_by: %d}\n",
           genesis_block.height, genesis_block.timestamp, genesis_block.hash, genesis_block.prev_hash,
           genesis_block.difficulty, genesis_block.nonce, genesis_block.relayed_by);

    // Create server pipe
    mkfifo(SERVER_PIPE_PATH, 0666);
    printf("Listening on server_pipe\n");
    int server_pipe_fd = open(SERVER_PIPE_PATH, O_RDONLY);
    if (server_pipe_fd < 0) {
        perror("server_pipe_fd");
        exit(1);
    }

    while (1) {
        int miner_pipe_fd;
        BLOCK_T buffer_block;
        int bytes_read = read(server_pipe_fd, &buffer_block, sizeof(BLOCK_T));
        if (bytes_read > 0) {
            if (strncmp((char *) &buffer_block, MINER_PIPE_PATH, strlen(MINER_PIPE_PATH)) == 0) {
                // New miner subscription
                char *miner_id = (char *) &buffer_block + strlen(MINER_PIPE_PATH);
                char pipe_name[strlen(MINER_PIPE_PATH) + 2];
                strcpy(pipe_name, MINER_PIPE_PATH);
                strcat(pipe_name, miner_id);
                printf("Received connection request from miner id %s, pipe name %s\n", miner_id, pipe_name);

                miner_pipe_fd = open(pipe_name, O_WRONLY);
                if (miner_pipe_fd < 0) {
                    perror("miner_pipe_fd");
                    exit(1);
                }
                int bytes_write = write(miner_pipe_fd, &latest_block, sizeof(BLOCK_T));
                if (bytes_write < 0) {
                    printf("server couldn't write to miner\n");
                    // Print the error message
                    perror("write");
                    // Optionally, you can also print the value of errno and its corresponding error string
                    printf("Error number: %d\n", errno);
                    printf("Error description: %s\n", strerror(errno));
                    exit(1);
                }
                close(miner_pipe_fd);
            }
            else {
                // New block received from miner
                BLOCK_T new_block;
                memcpy(&new_block, &buffer_block, sizeof(BLOCK_T));
                printf("Server: New block received by miner %d, attributes: height: %d, timestamp: %d, hash: 0x%x, prev_hash: 0x%x, difficulty: %d, nonce: %d\n",
                       new_block.relayed_by, new_block.height, new_block.timestamp, new_block.hash, new_block.prev_hash,
                       new_block.difficulty, new_block.nonce);

                //get num miners
                const char *filePath = MINER_COUNT;
                FILE *file = fopen(filePath, "r");
                if (file == NULL) {
                    perror("fopen");
                    exit(1);
                }


                int num_miners;
                if (fscanf(file, "%d", &num_miners) != 1) {
                    perror("fscanf");
                    fclose(file);
                    exit(1);
                }


                fclose(file);


                //////////////////////


                if (verify_difficulty(new_block.hash) && new_block.height == block_count && new_block.prev_hash == latest_block.hash) {
                    latest_block = new_block; // Update the latest block
                    block_count++; // Add 1 to the block count
                    printf("Server: New block added: {height: %d, timestamp: %d, hash: 0x%x, prev_hash: 0x%x, difficulty: %d, nonce: %d, relayed_by: %d}\n",
                           new_block.height, new_block.timestamp, new_block.hash, new_block.prev_hash,
                           new_block.difficulty, new_block.nonce, new_block.relayed_by);


                    for (int i = 1; i <= num_miners; i++) {
                        char miner_pipe_name[256];
                        sprintf(miner_pipe_name, "%s%d", MINER_PIPE_PATH, i);
                        miner_pipe_fd = open(miner_pipe_name, O_WRONLY);
                        if (miner_pipe_fd == -1) {
                            perror("open");
                            exit(1);
                        }
                        if (write(miner_pipe_fd, &latest_block, sizeof(BLOCK_T)) == -1) {
                            perror("write");
                            close(miner_pipe_fd);
                            exit(1);
                        }
                        printf("sending latest block number: %d to miner %d\n", latest_block.height, i);
                    }
                }
                else {
                    printf("Invalid block received from miner %d\n", new_block.relayed_by);
                }
            }
        }
    }
}


int main() {
    create_log_file();
    difficulty = get_difficulty();
    server_function();
    return 0;
}

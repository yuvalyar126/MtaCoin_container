**DOCKER USERNAME - yuvalyar12**


**Project: Blockchain Simulation - MtaCoin**

**Description:**

This project simulates a simplified blockchain network with a server and multiple miner nodes. The server is responsible for managing the blockchain and validating new blocks, while the miners compete to mine new blocks by solving cryptographic puzzles.

**Features**

Uses named pipes for inter-process communication between the server and miners.
Ensures concurrency control and synchronization while miners are writing to the shared blockchain.
Implements a simple proof-of-work mechanism by adjusting the nonce to meet the difficulty target.

1. Server (server.c):

Manages the blockchain by validating and appending new blocks.
Listens for connection requests from miners and distributes the latest block information.
Verifies that the difficulty and previous hash of new blocks match the expected values before adding them to the blockchain.

2. Miner (miner.c):

Connects to the server to receive the latest block information.
Mines new blocks by incrementing a nonce until the block hash meets the difficulty criteria.
Sends new valid blocks to the server for verification and inclusion in the blockchain.

**Features**

Uses named pipes for inter-process communication between the server and miners.
Ensures concurrency control and synchronization while miners are writing to the shared blockchain.
Implements a proof-of-work mechanism by adjusting the nonce to meet the difficulty target.

**Requirements**

A C compiler (like GCC).

POSIX-compliant system (Linux/Unix) for named pipe usage.

NOTE - Please delete miner_count.txt before each run (execpt the first).
NOTE - You might encounter an error when running, please try running the program a few times. It should work after a few attempts. 

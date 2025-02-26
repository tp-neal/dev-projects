
/***************************************************************************************************
* @project: RPC System Calls
****************************************************************************************************
* @file user1.c
* @brief Simulated user who opens a remote file, and copies it to a local file.
*
* This program simulates a user connecting to the server. The user then performs
* the following operations:
*   1. Sets up a socket connection with the server
*   2. Opens a remote file for reading
*   3. Requests a checksum of the remote file
*   4. Creates a local file for copying the remotefile into
*   5. Copies the remotefile to local directory
*   6. Closes the remote file
*   7. Computes a checksum for the local file copy
*   8. Compares the remote checksum to the local to verify file integrity
*   9. Prints whether the checksums match
*
* @author Tyler Neal
* @date 2/26/2025
***************************************************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "util.h"

/*==================================================================================================
    Main
==================================================================================================*/

/**
 * @brief Demonstrates a client connection to the server
 *
 * @param argc Number of command-line arguments
 * @param argv Command-line arguments
 *             arg[1] hostname - IPv4 address of host
               arg[2] port - port number to connect to host on
               arg[3] remote_file_path - path to the remote text file
               arg[4] local_file_path - path to local file to create
 * @return 0 on successful execution, -1 on error
 */
int main(int argc, char** argv) {

    // Verify argument count
    if (argc != 5) {
        fprintf(stderr, "Usage: <hostname> <port> "
                                       "<remote_file_path> <local_file_path>\n");
        return -1;
    }

    // Retrieve arguments
    char* hostname = argv[1];
    int port = atoi(argv[2]);
    char* remote_file_path = argv[3];
    char* local_file_path = argv[4];

    // Connect to server
    int server_fd;
    int status = rp_connect(&server_fd, port, hostname);
    if (status < 0) {
        perror("[User : Error] - failed to connect to server");
        return -1;
    } 
    printf("[User : Info] Connected to server on port %d\n", port);

    // Open remote file
    printf("[User : Info] Opening remote file: %s\n", remote_file_path);
    int remote_fd = rp_open(server_fd, remote_file_path, O_RDONLY);
    if (remote_fd < 0) {
        perror("[User : Error] - failed to open remote file");
        return -1;
    }
    printf("[User : Info] Remote file opened successfully (fd: %d)\n", remote_fd);

    // Request checksum of remote file
    printf("[User : Info] Computing remote file checksum...\n");
    short remote_checksum = rp_checksum(server_fd, remote_fd, CHECKSUM_BLOCK_SIZE);
    if (remote_checksum == -1) {
        perror("[User : Error] - failed to get checksum for remote file");
        return -1;
    }
    printf("[User : Info] Remote checksum: %hd\n", remote_checksum);

    // Open local file
    printf("[User : Info] Creating local file: %s\n", local_file_path);
    int local_file = open(local_file_path, O_CREAT | O_RDWR, 0744);
    if (local_file < 0) {
        perror("[User : Error] - failed to open local file");
        return -1;
    }

    // Allocate buffer for reading from server/writing to local file
    char* buffer = (char*)malloc(USER_BUFFER_SIZE);
    if (!buffer) {
        perror("[User : Error] - failed to allocate buffer");
        return -1;
    }
    
    // Write the contents of the remote file to local file
    printf("[User : Info] Copying data from remote to local file...\n");
    ssize_t bytes_read, bytes_wrote, total_bytes_copied = 0;
    while ((bytes_read = rp_read(server_fd, remote_fd, buffer, USER_BUFFER_SIZE)) > 0) {
        printf("*** Read in %zu bytes ***\n", bytes_read);
        if ((bytes_wrote = write(local_file, buffer, bytes_read)) == -1) {
            perror("[User : Error] - failed to write to local file");
            free(buffer);
            return -1;
        }
        total_bytes_copied += bytes_wrote;
        printf("*** Wrote %zu bytes ***\n", bytes_wrote);
    } 
    printf("[User : Info] Copy complete (%zd bytes transferred)\n", total_bytes_copied);
    free(buffer);

    // Catch read error
    if (bytes_read == -1) {
        perror("[User : Error] - failed to read from remote file");
        return -1;
    }

    // Close the remote file
    printf("[User : Info] Closing remote file: %d\n", remote_fd);
    if (rp_close(server_fd, remote_fd) < 0) {
        perror("[User : Error] Failed to read from remote file");
        return -1;
    }

    // Compute checksum of copied file to verify integrity
    printf("[User : Info] Computing local file checksum...\n");
    short local_checksum = genChecksum(local_file, CHECKSUM_BLOCK_SIZE);
    if (local_checksum == -1) {
        perror("[User : Error] - failed to generate checksum for local file");
        return -1;
    }
    printf("[User : Info] Local checksum: %hd\n", local_checksum);

    // Print status of copy
    if (remote_checksum == local_checksum) {
        printf("[User : Info] SUCCESS: File copied successfully (Checksums match: %hd)\n", local_checksum);
    } else {
        printf("[User : Error] ERROR: File copy validation failed (Remote: %hd, Local: %hd)\n", 
               remote_checksum, local_checksum);
        return -1;
    }

    return 0;
}
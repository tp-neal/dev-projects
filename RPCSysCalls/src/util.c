
/***************************************************************************************************
* @project: RPC System Calls
****************************************************************************************************
* @file util.c
* @brief Contains utility function definitions used in both the client and the
*        server.
*
* @author Tyler Neal
* @date 2/26/2025
***************************************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "error.h"
#include "protocol.h"
#include "util.h"

/*==================================================================================================
    Server & Client Reading/Writing
==================================================================================================*/

/**
 * @brief Reads data from a file descriptor. First the size of the data is
 * read in as a size_t, and then a buffer is allocated to read in the actual
 * data contents
 * 
 * @param fd File descriptor to read from
 * @return void* Pointer to the data read, NULL on error with errno set
 * @note Caller is responsible for freeing the returned buffer
 */
void* read_from_connection(int fd) {
    size_t data_size;
    ssize_t bytes_read;

    // Read in the size of data
    bytes_read = read(fd, &data_size, sizeof(data_size));
    if (bytes_read == -1) {
        return NULL;
    }
    if (bytes_read == 0) {
        errno = CONNECTION_CLOSED;
        return NULL;
    }

    // Convert size to host byte order
    data_size = ntohl(data_size);

    // Allocate buffer for data
    void* buffer = malloc(data_size);
    if (!buffer) {
        return NULL;
    }

    // Read in the data of specified size
    bytes_read = read(fd, buffer, data_size);
    if (bytes_read == -1) {
        return NULL;
    }
    if (bytes_read == 0) {
        errno = CONNECTION_CLOSED;
        return NULL;
    }

    return buffer;
}

/**
 * @brief Sends data over a file descriptor with size prefix
 * 
 * @param fd File descriptor to write to
 * @param data Pointer to the data to be sent
 * @param data_size Size of the data in bytes
 * @return 0 on success, -1 on error with errno set
 */
int send_to_connection(int fd, void* data, size_t data_size) {
    size_t data_size_net = htonl(data_size);

    // Write data size to client
    if (write(fd, &data_size_net, sizeof(data_size_net)) == -1) {
        return -1;
    }

    // Write data to client
    if (write(fd, data, data_size) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @brief Reads a parameter of specified type from specified connection
 * 
 * @param var Pointer to store the read parameter
 * @param type Type of the parameter (INT32, UINT32, INT16)
 * @param client_fd Client connection file descriptor
 * @return 0 on success, -1 on error with errno set
 */
int read_data_of_type(void* var, var_type type, int client_fd) {
    switch (type) {
        case INT32: {
            int32_t* int_ptr = (int32_t*)read_from_connection(client_fd);
            if (!int_ptr) // break if value want read
                return -1;
            *(int32_t*)var = ntohl(*int_ptr);
            free(int_ptr);
            break;
        }
        case UINT32: {
            uint32_t* uint_ptr = (uint32_t*)read_from_connection(client_fd);
            if (!uint_ptr) // break if value want read
                return -1;
            *(uint32_t*)var = ntohl(*uint_ptr);
            free(uint_ptr);
            break;
        }
        case INT16: {
            int16_t* short_ptr = (int16_t*)read_from_connection(client_fd);
            if (!short_ptr) // break if value want read
                return -1;
            *(int16_t*)var = ntohs(*short_ptr);
            free(short_ptr);
            break;
        }
        default:
            errno = INVALID_VARIABLE_TYPE;
            return -1;
    }
    return 0;
}

/**
 * @brief Sends a result of specified type to specified fd
 * 
 * @param result Pointer to the result to send
 * @param type Type of the result (INT32, UINT32, INT16)
 * @param client_fd Client connection file descriptor
 * @return 0 on success, -1 on error with errno set
 */
int send_data_of_type(void* result, var_type type, int client_fd) {
    // We cast signed integers to unsigned because bit integrity remains same
    switch (type) {
        case INT32: {
            uint32_t copy;
            memcpy(&copy, result, sizeof(copy));
            uint32_t net_val = htonl(copy);
            if (send_to_connection(client_fd, &net_val, sizeof(net_val)) < 0) {
                errno = SERVER_ERROR_SENDING_RPC_RESULT;
                return -1;
            }
            break;
        }
        case UINT32: {
            uint32_t copy;
            memcpy(&copy, result, sizeof(copy));
            uint32_t net_val = htonl(copy); // Use 64-bit conversion
            if (send_to_connection(client_fd, &net_val, sizeof(net_val)) < 0) {
                errno = SERVER_ERROR_SENDING_RPC_RESULT;
                return -1;
            }
            break;
        }
        case INT16: {
            uint16_t copy;
            memcpy(&copy, result, sizeof(copy));
            uint16_t net_val = htons(copy); // Use 16-bit conversion
            if (send_to_connection(client_fd, &net_val, sizeof(net_val)) < 0) {
                errno = SERVER_ERROR_SENDING_RPC_RESULT;
                return -1;
            }
            break;
        }
        default:
            errno = SERVER_INVALID_RESULT_TYPE;
            return -1;
    }
    return 0;
}

/**
 * @brief Converts a call type code to its string representation
 * 
 * @param call_type Integer representing the call type
 * @return String representation of the call type
 */
char* strCallType(int call_type) {

    switch(call_type) {
        case OPEN_CALL:
            return "OPEN";
        case CLOSE_CALL:
            return "CLOSE";
        case READ_CALL:
            return "READ";
        case WRITE_CALL:
            return "WRITE";
        case LSEEK_CALL:
            return "LSEEK";
        case CHECKSUM_CALL:
            return "CHECKSUM";
        default:
            return "INVALID";
    }
}

/**
 * @brief Generates a checksum for a file by XORing bytes
 * 
 * @param fd File descriptor of the file
 * @param block_size Size of blocks to read at a time
 * @return Checksum value, -1 on error with errno set
 * @note Resets file position to start after completion
 */
short genChecksum(int fd, int block_size) {

    short checksum = 0;

    // Return cursor to start of file
    if (lseek(fd, 0, SEEK_SET) < 0)
        return -1;

    // Allocate buffer
    uint8_t* buffer = malloc(block_size);
    if (!buffer) {
        return -1;
    }

    // Computer checksum by xoring bytes
    int bytes_read;
    while ((bytes_read = read(fd, buffer, block_size)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            checksum ^= buffer[i];
        }
    }

    // Free the read buffer
    free(buffer);

    // If read error return
    if (bytes_read == -1)
        return -1;

    // Return cursor to start of file
    if (lseek(fd, 0, SEEK_SET) < 0)
        return -1;

    return checksum;
}
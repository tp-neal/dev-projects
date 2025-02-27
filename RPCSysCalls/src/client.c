
/***************************************************************************************************
* @project: RPC System Calls
****************************************************************************************************
* @file client.c
* @brief Implementations of the client rpc request functions.
*
* These functions act as a means to request a system call is performed on the
* remote server. The client begins by sending arguments to the server, after 
* which, the server will send back the result of the operation, as well as the
* errno if an error was encountered.
*
* @author Tyler Neal
* @date 2/26/2025
***************************************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "client.h"
#include "error.h"
#include "protocol.h"
#include "util.h"

/*==================================================================================================
    Connection Setup
==================================================================================================*/

/**
 * @brief Connects to a remote server
 * 
 * @param socket_fd Pointer to store the created socket file descriptor
 * @param port Server port to connect to
 * @param hostname Server hostname or IP address
 * @return RP_SUCCESS on success, error code on failure
 */
int rp_connect(int* socket_fd, int port, char* hostname) {

    // Get file descriptor for the socket
    *socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*socket_fd < 0) {
        fprintf(stderr, "[Client] - socket creation failed: %s\n", strerror(errno));
        return CLIENT_CREATE_SOCKET_ERROR;
    }

    // Fill in the address member variables
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    // Get the hostname
    inet_pton(AF_INET, hostname, &address.sin_addr);

    if (connect(*socket_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        fprintf(stderr, "[Client] - failed to connect to server: %s\n", strerror(errno));
        return CLIENT_CONNECTION_ERROR;
    }

    return RP_SUCCESS;
}

/*==================================================================================================
    RPC Callers
==================================================================================================*/

/**
 * @brief Remote procedure call for open system call
 * 
 * @param server_fd Server connection file descriptor
 * @param pathname Path of the file to open
 * @param flags Open flags (O_RDONLY, O_WRONLY, etc.)
 * @param ... Optional mode argument for O_CREAT
 * @return File descriptor on success, -1 on error with errno set
 */
int32_t rp_open(int server_fd, char* pathname, int flags, ...) {
    // Set errno for early exit
    errno = CLIENT_ERROR_SENDING_RPC_ARGS;
    
    // Send call type to server
    uint32_t call_code_net = htonl(OPEN_CALL);
    if (send_to_connection(server_fd, &call_code_net, sizeof(call_code_net)) == -1)
        return -1;

    // Send pathname
    if (send_to_connection(server_fd, pathname, strlen(pathname)+1) == -1)
        return -1;

    // Send flags
    uint32_t flags_net = htonl((uint32_t)flags);
    if (send_to_connection(server_fd, &flags_net, sizeof(flags_net)) == -1)
        return -1;

    // Send mode if creating a new file
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        uint32_t mode_net = htonl((uint32_t)mode);
        if (send_to_connection(server_fd, &mode_net, sizeof(mode_net)) == -1)
            return -1;
    }

    // Recieve result from server
    int32_t result;
    if (recieve_result(server_fd, &result, INT32) == -1)
        return -1;

    return result;
}

/**
 * @brief Remote procedure call for close system call
 * 
 * @param server_fd Server connection file descriptor
 * @param file_fd File descriptor to close
 * @return 0 on success, -1 on error with errno set
 */
int32_t rp_close(int server_fd, int file_fd) {
    // Set errno for early exit
    errno = CLIENT_ERROR_SENDING_RPC_ARGS;
    
    // Send call type to server
    uint32_t call_code_net = htonl(CLOSE_CALL);
    if (send_to_connection(server_fd, &call_code_net, sizeof(call_code_net)) == -1)
        return -1;

    // Send file descriptor
    uint32_t file_fd_net = htonl((uint32_t)file_fd);
    if (send_to_connection(server_fd, &file_fd_net, sizeof(file_fd_net)) == -1)
        return -1;

    // Recieve result from server
    int32_t result;
    if (recieve_result(server_fd, &result, INT32) == -1) 
        return -1;

    return result;
}

/**
 * @brief Remote procedure call for read system call
 * 
 * @param server_fd Server connection file descriptor
 * @param file_fd File descriptor to read from
 * @param buffer Buffer to store read data
 * @param count Maximum number of bytes to read
 * @return Number of bytes read, 0 at EOF, -1 on error with errno set
 */
int32_t rp_read(int server_fd, int file_fd, char* buffer, size_t count) {
    // Set errno for early exit
    errno = CLIENT_ERROR_SENDING_RPC_ARGS;

    // Send call type to server
    uint32_t call_code_net = htonl(READ_CALL);
    if (send_to_connection(server_fd, &call_code_net, sizeof(call_code_net)) == -1)
        return -1;

    // Send file descriptor
    uint32_t file_fd_net = htonl((uint32_t)file_fd);
    if (send_to_connection(server_fd, &file_fd_net, sizeof(file_fd_net)) == -1)
        return -1;

    // Send buffer
    if (send_to_connection(server_fd, buffer, count) == -1)
        return -1;

    // Send count
    uint32_t count_net = htonl((uint32_t)count);
    if (send_to_connection(server_fd, &count_net, sizeof(count_net)) == -1)
        return -1;

    // Recieve result from server
    int32_t data_read;
    if (recieve_result(server_fd, &data_read, INT32) == -1) 
        return -1;

    // Copy in data read
    if (data_read > 0) {
        char* remote_buffer = (char*)read_from_connection(server_fd);
        memcpy(buffer, remote_buffer, data_read);
        free(remote_buffer);
    }
   

    return data_read;
}

/**
 * @brief Remote procedure call for write system call
 * 
 * @param server_fd Server connection file descriptor
 * @param file_fd File descriptor to write to
 * @param buffer Buffer containing data to write
 * @param count Number of bytes to write
 * @return Number of bytes written, -1 on error with errno set
 */
int32_t rp_write(int server_fd, int file_fd, char* buffer, size_t count) {
    // Set errno for early exit
    errno = CLIENT_ERROR_SENDING_RPC_ARGS;

    // Send call type to server
    uint32_t call_code_net = htonl(WRITE_CALL);
    if (send_to_connection(server_fd, &call_code_net, sizeof(call_code_net)) == -1)
        return -1;

    // Send file descriptor
    uint32_t file_fd_net = htonl((uint32_t)file_fd);
    if (send_to_connection(server_fd, &file_fd_net, sizeof(file_fd_net)) == -1)
        return -1;

    // Send buffer
    if (send_to_connection(server_fd, buffer, count) == -1)
        return -1;

    // Send count
    uint32_t count_net = htonl((uint32_t)count);
    if (send_to_connection(server_fd, &count_net, sizeof(count_net)) == -1)
        return -1;

    // Recieve result from server
    int32_t data_wrote;
    if (recieve_result(server_fd, &data_wrote, INT32) == -1)
        return -1;

    return data_wrote;
}

/**
 * @brief Remote procedure call for lseek system call
 * 
 * @param server_fd Server connection file descriptor
 * @param file_fd File descriptor to seek on
 * @param offset Offset for the seek operation
 * @param whence Base position for seek (SEEK_SET, SEEK_CUR, SEEK_END)
 * @return New file offset on success, -1 on error with errno set
 */
int32_t rp_lseek(int server_fd, int file_fd, off_t offset, int whence) {
    // Set errno for early exit
    errno = CLIENT_ERROR_SENDING_RPC_ARGS;

    // Send call type to server
    uint32_t call_code_net = htonl(LSEEK_CALL);
    if (send_to_connection(server_fd, &call_code_net, sizeof(call_code_net)) == -1)
        return -1;

    // Send file descriptor
    uint32_t file_fd_net = htonl((uint32_t)file_fd);
    if (send_to_connection(server_fd, &file_fd_net, sizeof(file_fd_net)) == -1)
        return -1;

    // Send offset
    uint32_t offset_net = htonl((uint32_t)offset);
    if (send_to_connection(server_fd, &offset_net, sizeof(offset_net)) == -1)
        return -1;

    // Send lseek origin
    uint32_t whence_net = htonl((uint32_t)whence);
    if (send_to_connection(server_fd, &whence_net, sizeof(whence_net)) == -1)
        return -1;

    // Recieve result from server
    int32_t result;
    if (recieve_result(server_fd, &result, INT32) == -1)
        return -1;

    return result;
}

/**
 * @brief Remote procedure call to generate a checksum for a file
 * 
 * @param server_fd Server connection file descriptor
 * @param file_fd File descriptor to generate checksum for
 * @param block_size Size of blocks for checksum calculation
 * @return Calculated checksum value, -1 on error with errno set
 */
int16_t rp_checksum(int server_fd, int file_fd, size_t block_size) {
    // Set errno for early exit
    errno = CLIENT_ERROR_SENDING_RPC_ARGS;

    // Send call type to server
    uint32_t call_code_net = htonl(CHECKSUM_CALL);
    if (send_to_connection(server_fd, &call_code_net, sizeof(call_code_net)) < 0)
        return -1;

    // Send file descriptor
    uint32_t file_fd_net = htonl((uint32_t)file_fd);
    if (send_to_connection(server_fd, &file_fd_net, sizeof(file_fd_net)) < 0)
        return -1;

    // Send block_size
    uint32_t block_size_net = htonl((uint32_t)block_size);
    if (send_to_connection(server_fd, &block_size_net, sizeof(block_size_net)) < 0)
        return -1;

    // Recieve result from server
    int16_t checksum;
    if (recieve_result(server_fd, &checksum, INT16) < 0)
        return -1;
    
    return checksum;
}

/*==================================================================================================
    Client Specific Helpers
==================================================================================================*/

/**
 * @brief Updates the local errno from the server's errno
 * 
 * @param server_fd Server connection file descriptor
 * @return 0 on success, -1 on error
 */
 int update_errno(int server_fd) {
    uint32_t* errno_ptr = (uint32_t*)read_from_connection(server_fd);
    if (!errno_ptr)
        return -1;
    errno = (int)ntohl(*errno_ptr);
    return 0;
}

/**
 * @brief Recieve the return of a system call from the server, and an error code if an error 
 *        occured.
 * 
 * @param server_fd File descriptor of the server connection
 * @param result_buffer Numberic variable to load the recieved syscall result into
 * @param type Type of variable being returned
 * @param read_buffer Buffer for read data during read calls, otherwise NULL
 * @return int 0 on success : 1 on error
 */
int recieve_result(int server_fd, void* result_buffer, var_type_e type) {

    // Retrieve result from server
    if (read_data_of_type(result_buffer, type, server_fd) == -1) {
        errno = CLIENT_ERROR_RECIEVING_RPC_RESULT;
        return -1;
    }

    // Dereference the result
    int32_t result;
    memcpy(&result, result_buffer, sizeof(result));

    // Update errno if neccessary
    if (result == -1) {
        if (update_errno(server_fd) == -1) {
            errno = CLIENT_ERROR_RECIEVING_RPC_ERRNO;
            return -1;
        }
    }

    return 0;
}

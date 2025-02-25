
/*******************************************************************************
* @project: RPC System Calls
********************************************************************************
* @file server.c
* @brief Connects to clients and performs remote protocol system calls.
*
* The server performs system calls request by remote clients, and then provides
* the results back to them. The server performs the following operations:
*   1. Sets up a socket listening on the specified port
*   2. Accepts client connections
*   3. Forks to handle each client in a separate process
*   4. Processes remote procedure calls from the client
*   5. Executes system calls on behalf of the client
*   6. Returns the results of the system call to the client
*
* @author Tyler Neal
* @date 2/23/2025
*******************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "error.h"
#include "protocol.h"
#include "server.h"
#include "util.h"

static int socket_fd;

/*==============================================================================
                                   Main
==============================================================================*/

/**
 * @brief Main entry point for the server application
 *
 * @param argc Number of command-line arguments
 * @param argv Command-line arguments (expects port number)
 * @return 0 on successful execution, -1 on error
 */
int main(int argc, char** argv) {

    // Verify argument count
    if (argc != 2) {
        fprintf(stderr, "Usage: <port>\n");
        return -1;
    }

    // Retrieve arguments
    int port = atoi(argv[1]);
    struct sockaddr_in address;

    // Create a handler for interrupts
    if (signal(SIGINT, interrupt_handler) == SIG_ERR) {
        perror("[Server : Error] Failed to register signal handler");
        return -1;
    }

    // Setup the server socket
    if (setupServer(&socket_fd, &address, port) == -1) {
        perror("[Server : Error] Failed during setupServer()");
        return -1;
    }

    // Create container for accepts() address length field
    int address_length = sizeof(address);

    // Continuously accept connections
    while (1) {
        int connection_fd = accept(socket_fd, 
                             (struct sockaddr*)&address, 
                         (socklen_t*)&address_length);
        if (connection_fd == -1) {
            perror("[Server : Error] Error while accepting connection");
            continue;
        }
        printf("[Server : Info] "
               "New client connection accepted (fd: %d)\n", 
               connection_fd);

        // Fork into child to handle request
        pid_t pid = fork();

        // Catch error
        if (pid == -1) {
            perror("[Server : Error] Error while forking");
            close(connection_fd);
            break;
        }

        // Parent closes connection and goes back to accepting
        if (pid > 0) {
            printf("[Server : Info] "
                   "Forked child process (pid: %d) to handle client request\n", 
                   pid);
            close(connection_fd);
            continue;
        }


        /* CHILD CODE FOLLOWS */
        close(socket_fd);

        // Define call for error printing
        int status = RP_SUCCESS; // holds status of child
        char* call_str;

        // Handle requests from client
        while(1) {
            
            // Retreive call type
            int call_type;
            int* call_type_ptr = read_from_connection(connection_fd);
            if (call_type_ptr) {
                call_type = ntohl(*call_type_ptr);
                call_str = strCallType(call_type);
            } else {
                if (errno == CONNECTION_CLOSED) {
                    fprintf(stderr, "[Server : Warning] "
                            "Client closed connection\n");
                } else {
                    fprintf(stderr, "[Server : Error] "
                            "Failed to read call_type from client\n");
                    errno = SERVER_CALL_TYPE_READ_ERROR;
                    status = -1;
                }
                break;
            }

            printf("[Server Child : Info] Processing request: %s\n", 
                   call_str);

            // Handle call
            switch(call_type) {
                
                case OPEN_CALL:
                    status = handle_open(connection_fd);
                    break;

                case CLOSE_CALL:
                    status = handle_close(connection_fd);
                    break;

                case READ_CALL:
                    status = handle_read(connection_fd);
                    break;

                case WRITE_CALL:
                    status = handle_write(connection_fd);
                    break;

                case LSEEK_CALL:
                    status = handle_lseek(connection_fd);
                    break;

                case CHECKSUM_CALL:
                    status = handle_checksum(connection_fd);
                    break;
                
                default:
                    errno = SERVER_INVALID_CALL_TYPE;
                    break;
            }

            // Terminate if error status or closed connection
            if (status == -1 || errno == CONNECTION_CLOSED) {
                break;
            }
        }

        // Server child exit
        close(socket_fd);
        close(connection_fd);
        fprintf(stderr, "[Server Child : Info] "
                "terminating with status code {%s: errno[%d]}\n", 
                call_str, errno);
        return status;
    }

    // Server parent exit
    return 0; // (this is dead code as parent runs until signal interrupt)
}

/*==============================================================================
                           Server Setup / Handling
==============================================================================*/

/**
 * @brief Signal handler for handling interrupt signals
 * 
 * @param sig_number Signal number that triggered the handler
 */
void interrupt_handler(int sig_number) {
    fprintf(stderr, "Recieved a SIGNAL INTERRUPT: %d, "
            "exiting...\n", sig_number);
    close(socket_fd);
    exit(1);
}

/**
 * @brief Sets up the server socket for listening
 * 
 * @param socket_fd Pointer to store the created socket file descriptor
 * @param address Pointer to store socket address information
 * @param port Port number to listen on
 * @return 0 on success, -1 on error with errno set
 */
int setupServer(int* socket_fd, struct sockaddr_in* address, int port) {

    printf("[Server : Info] Starting RPC server on port %d\n", port);

    // Get socket file descriptor
    *socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*socket_fd  < 0) {
        return -1;
    }

    // Populate address struct
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(port);

    // Attempt to bind socket
    if (bind(*socket_fd, 
       (struct sockaddr *)address, 
        sizeof(*address)) < 0) 
    {
        perror("[Server : Error] Socket bind failed");
        close(*socket_fd);  // clean up socket
        return -1;
    }

    // Attempt to listen to socket
    if (listen(*socket_fd, BACKLOG_SIZE) < 0) {
        perror("[Server : Error] Socket listen failed");
        close(*socket_fd);  // clean up socket
        return -1;
    }

    printf("[Server : Info] Server initialized and listening on port %d\n", port);

    return RP_SUCCESS;
}

/*==============================================================================
                               RPC Handlers
==============================================================================*/

/**
 * @brief Handles an open system call from the client
 * 
 * @param client_fd Client connection file descriptor
 * @return 0 on success, -1 on error with errno set
 */
int handle_open(int client_fd) {
    // Set errno for early exit
    errno = SERVER_ERROR_RECIEVING_RPC_ARGS;

    // Get file descriptor
    char* pathname = (char*)read_from_connection(client_fd);
    if (!pathname) {
        return -1;
    }

    // Get flags
    uint32_t flags;
    if (read_data_of_type(&flags, UINT32, client_fd) == -1) {
        free(pathname);
        return -1;
    }

    // Get mode if necessary
    uint32_t mode;
    if (flags & O_CREAT) {
        if (read_data_of_type(&mode, UINT32, client_fd) == -1){
            free(pathname);
            return -1;
        }
    }

    // Perform call
    errno = 0;
    int32_t result = (flags & O_CREAT) ?
        (int32_t) open(pathname, (int)flags, (mode_t)mode) :
        (int32_t) open(pathname, (int)flags);

    // Return the result (INT32 due to potential to be negative)
    if (send_data_of_type(&result, INT32, client_fd) == -1) {
        errno = SERVER_ERROR_SENDING_RPC_RESULT;
        return -1;
    }

    // Send error number if necessary
    if (result == -1) {
        if (send_data_of_type(&errno, INT32, client_fd) == -1) {
            errno = SERVER_ERROR_SENDING_RPC_ERRNO;
            return -1;
        }
    }
    
    free(pathname);
    return RP_SUCCESS;
}

/**
 * @brief Handles a close system call from the client
 * 
 * @param client_fd Client connection file descriptor
 * @return 0 on success, -1 on error with errno set
 */
int handle_close(int client_fd) {
    // Set errno for early exit
    errno = SERVER_ERROR_RECIEVING_RPC_ARGS;

    // Get file descriptor
    uint32_t file_fd;
    if (read_data_of_type(&file_fd, UINT32, client_fd) == -1)
        return -1;

    // Perform call and return results
    errno = 0;
    int32_t result = (int32_t)close(file_fd);
    if (send_data_of_type(&result, INT32, client_fd) < 0)
        return -1;

    // Send error number if necessary
    if (result == -1) {
        if (send_data_of_type(&errno, INT32, client_fd) == -1)
            return -1;
    }
    
    return RP_SUCCESS;
}

/**
 * @brief Handles a read system call from the client
 * 
 * @param client_fd Client connection file descriptor
 * @return 0 on success, -1 on error with errno set
 */
int handle_read(int client_fd) {
    // Set errno for early exit
    errno = SERVER_ERROR_RECIEVING_RPC_ARGS;

    // Get file descriptor
    uint32_t file_fd;
    if (read_data_of_type(&file_fd, UINT32, client_fd) == -1)
        return -1;

    // Get buffer
    char* buffer = (char*)read_from_connection(client_fd);
    if (!buffer)
        return -1;

    // Get count
    uint32_t count;
    if (read_data_of_type(&count, UINT32, client_fd) == -1) {
        free(buffer);    
        return -1;
    }

    // Perform call and return results
    errno = 0;
    int32_t data_read = (int32_t)read(file_fd, buffer, count);
    if (send_data_of_type(&data_read, INT32, client_fd) < 0)
        return -1;

    // Send back read data
    if (data_read > 0) {
        if (send_to_connection(client_fd, buffer, data_read) < 0)
            return -1;
    }

    // Send error number if necessary
    if (data_read == -1) {
        if (send_data_of_type(&errno, INT32, client_fd) == -1)
            return -1;
    }
    
    free(buffer);
    return RP_SUCCESS;
}

/**
 * @brief Handles a write system call from the client
 * 
 * @param client_fd Client connection file descriptor
 * @return 0 on success, -1 on error with errno set
 */
int handle_write(int client_fd) {
    // Set errno for early exit
    errno = SERVER_ERROR_RECIEVING_RPC_ARGS;

    // Get file descriptor
    uint32_t file_fd;
    if (read_data_of_type(&file_fd, UINT32, client_fd) == -1)
        return -1;

    // Get buffer
    char* buffer = (char*)read_from_connection(client_fd);
    if (!buffer)
        return -1;

    // Get count
    uint32_t count;
    if (read_data_of_type(&count, UINT32, client_fd) == -1) {
        free(buffer);    
        return -1;
    }

    // Perform call and return results
    errno = 0;
    int32_t data_wrote = (int32_t)write(file_fd, buffer, count);
    if (send_data_of_type(&data_wrote, INT32, client_fd) < 0)
        return -1;

    // Send back read data
    if (data_wrote > 0) {
        if (send_to_connection(client_fd, buffer, data_wrote) < 0)
            return -1;
    }

    // Send error number if necessary
    if (data_wrote == -1) {
        if (send_data_of_type(&errno, INT32, client_fd) == -1)
            return -1;
    }
    
    free(buffer);
    return RP_SUCCESS;
}

/**
 * @brief Handles a lseek system call from the client
 * 
 * @param client_fd Client connection file descriptor
 * @return 0 on success, -1 on error with errno set
 */
int handle_lseek(int client_fd) {
    // Set errno for early exit
    errno = SERVER_ERROR_RECIEVING_RPC_ARGS;

    // Get file descriptor
    uint32_t file_fd;
    if (read_data_of_type(&file_fd, UINT32, client_fd) == -1)
        return -1;

    // Set errno for early exit
    errno = SERVER_ERROR_RECIEVING_RPC_ARGS;

    // Get offset
    int32_t offset;
    if (read_data_of_type(&offset, INT32, client_fd) == -1)
        return -1;

    // Get file descriptor
    uint32_t whence;
    if (read_data_of_type(&whence, UINT32, client_fd) == -1)
        return -1;

    // Perform call and return results
    errno = 0;
    int32_t result = (int32_t)lseek(file_fd, offset, whence);
    if (send_data_of_type(&result, INT32, client_fd) < 0)
        return -1;

    // Send error number if necessary
    if (result == -1) {
        if (send_data_of_type(&errno, INT32, client_fd) == -1)
            return -1;
    }

    return RP_SUCCESS;
}

/**
 * @brief Handles a checksum calculation request from the client
 * 
 * @param client_fd Client connection file descriptor
 * @return 0 on success, -1 on error with errno set
 */
int handle_checksum(int client_fd) {
    // Set errno for early exit
    errno = SERVER_ERROR_RECIEVING_RPC_ARGS;

    // Get file descriptor
    uint32_t file_fd;
    if (read_data_of_type(&file_fd, UINT32, client_fd) == -1)
        return -1;

    // Get file descriptor
    uint32_t block_size;
    if (read_data_of_type(&block_size, UINT32, client_fd) == -1)
        return -1;

    // Perform call and return results
    errno = 0;
    int16_t checksum = genChecksum(file_fd, block_size);
    if (send_data_of_type(&checksum, INT16, client_fd) < 0)
        return -1;

    // Send error number if necessary
    if (checksum == -1) {
        if (send_data_of_type(&errno, INT32, client_fd) == -1)
            return -1;
    }
    
    return RP_SUCCESS;
}
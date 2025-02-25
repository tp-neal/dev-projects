#ifndef CLIENT_H
#define CLIENT_H

/*******************************************************************************
* @project: RPC System Calls
********************************************************************************
* @file client.h
* @brief This file contains client-side function declarations and macros.
*
* @author Tyler Neal
* @date 2/23/2025
*******************************************************************************/

#include <stdint.h>
#include <unistd.h>

#include "util.h"

/*==============================================================================
                                   Macros
==============================================================================*/

#define USER_BUFFER_SIZE 1024 // size of buffer used to read/write from
                              // remote files

/*==============================================================================
                            Function Declarations
==============================================================================*/

/* Connection Setup */
int32_t rp_connect(int* socket_fd, int port, char* hostname);

/* Client Specific Helpers */
int32_t update_errno(int server_fd);

/* RPC Callers */
int32_t rp_open(int server_fd, char* pathname, int flags, ...);
int32_t rp_close(int server_fd, int file_fd);
int32_t rp_read(int server_fd, int file_fd, char* buffer, size_t count);
int32_t rp_write(int server_fd, int file_fd, char* buffer, size_t count);
int32_t rp_lseek(int server_fd, int file_fd, off_t offset, int whence);
int16_t rp_checksum(int server_fd, int file_fd, size_t block_size);

#endif // CLIENT_H
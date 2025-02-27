#ifndef SERVER_H
#define SERVER_H

/***************************************************************************************************
* @project: RPC System Calls
****************************************************************************************************
* @file server.h
* @brief This file contains server-side function declarations and macros.
*
* @author Tyler Neal
* @date 2/26/2025
***************************************************************************************************/

#include "util.h"

/*==================================================================================================
    Macros
==================================================================================================*/

#define BACKLOG_SIZE 5

/*==================================================================================================
    Function Declarations
==================================================================================================*/

/* Server Setup / Signal Hanlding */
void interrupt_handler(int sig_number);
int setupServer(int* socket_fd, struct sockaddr_in* address, int port);

/* RPC Handlers */
int handle_open(int client_fd);
int handle_close(int client_fd);
int handle_read(int client_fd);
int handle_write(int client_fd);
int handle_lseek(int client_fd);
int handle_checksum(int client_fd);

/* RPC Handler Helpers */
int return_result(int client_fd, void* result_ptr, var_type_e type);

#endif // SERVER_H
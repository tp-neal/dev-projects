#ifndef UTIL_H
#define UTIL_H

/***************************************************************************************************
* @project: RPC System Calls
****************************************************************************************************
* @file util.h
* @brief Contains utility function/macros used in both client and server.
*
* @author Tyler Neal
* @date 2/26/2025
***************************************************************************************************/

#include <stdlib.h>

/*==================================================================================================
    Macros
==================================================================================================*/

#define CHECKSUM_BLOCK_SIZE 2 // buffer size for checksums

/*==================================================================================================
    Enumerations
==================================================================================================*/

typedef enum {
    INT32,
    UINT32,
    INT16,
} var_type;

/*==================================================================================================
    Function Declarations
==================================================================================================*/

/* File Descriptor Reading/Writing */
void* read_from_connection(int fd);
int send_to_connection(int fd, void* data, size_t data_size);
int read_data_of_type(void* var, var_type type, int client_fd);
int send_data_of_type(void* result, var_type type, int client_fd);

/* Additional Helpers */
char* strCallType(int call_type);
short genChecksum(int fd, int block_size);

#endif // UTIL_H
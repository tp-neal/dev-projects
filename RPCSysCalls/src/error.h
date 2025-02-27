#ifndef ERROR_H
#define ERROR_H

/***************************************************************************************************
* @project: RPC System Calls
****************************************************************************************************
* @file error.h
* @brief Contains error code information macros.
*
* @author Tyler Neal
* @date 2/26/2025
***************************************************************************************************/

/*==================================================================================================
    Enumerations
==================================================================================================*/

typedef enum {
    // Success codes (positive or zero)
    RP_SUCCESS = 0,
    
    // Connection status codes
    CONNECTION_CLOSED = -50,
    
    // General error codes (-100 to -199)
    ERR_SEND_DATA_OF_TYPE = -100,
    ERR_INVALID_VAR_TYPE = -101,
    
    // Server socket errors (-200 to -299)
    SERVER_CREATE_SOCKET_ERROR = -200,
    SERVER_BIND_SOCKET_ERROR = -201,
    SERVER_LISTEN_SOCKET_ERROR = -202,
    
    // Server operation errors (-300 to -399)
    SERVER_CALL_TYPE_READ_ERROR = -300,
    SERVER_INVALID_CALL_TYPE = -301,
    
    // Client errors (-400 to -499)
    CLIENT_CREATE_SOCKET_ERROR = -400,
    CLIENT_CONNECTION_ERROR = -401,
    CLIENT_FAILED_RP_OPEN = -402,
    
    // Server/Client data transfer errors (-500 to -599)
    SERVER_ERROR_RECIEVING_RPC_ARGS = -500,
    SERVER_ERROR_SENDING_RPC_RESULT = -501,
    SERVER_ERROR_SENDING_RPC_ERRNO = -502,
    CLIENT_ERROR_SENDING_RPC_ARGS = -503,
    CLIENT_ERROR_RECIEVING_RPC_RESULT = -504,
    CLIENT_ERROR_RECIEVING_RPC_ERRNO = -505,
    
    // Checksum related errors (-600 to -699)
    CHECKSUM_BUFFER_ALLOC_ERROR = -600,
    CHECKSUM_BAD_READ_ERROR = -601,
} error_code_e;


#endif // ERROR_H
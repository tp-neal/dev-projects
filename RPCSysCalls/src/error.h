#ifndef ERROR_H
#define ERROR_H

/*******************************************************************************
* @project: RPC System Calls
********************************************************************************
* @file error.h
* @brief Contains error code information macros.
*
* @author Tyler Neal
* @date 2/23/2025
*******************************************************************************/

/*==============================================================================
                                Enumarations
==============================================================================*/

typedef enum {
    ERROR = 1,
    WARNING = 2,
    INFO = 3,
    DEBUG = 4,
} message_type;

typedef enum {
    // Success codes (positive or zero)
    RP_SUCCESS = 0,

    // Connection status codes
    CONNECTION_CLOSED = -50,
    
    // Variable type validation
    INVALID_VARIABLE_TYPE = -99,

    // Server socket errors (-100 to -199)
    SERVER_CREATE_SOCKET_ERROR = -100,
    SERVER_BIND_SOCKET_ERROR = -101,
    SERVER_LISTEN_SOCKET_ERROR = -102,
    
    // Server operation errors (-200 to -299)
    SERVER_CALL_TYPE_READ_ERROR = -200,
    SERVER_INVALID_CALL_TYPE = -201,
    
    // Client errors (-300 to -399)
    CLIENT_CREATE_SOCKET_ERROR = -300,
    CLIENT_CONNECTION_ERROR = -301,
    CLIENT_FAILED_RP_OPEN = -302,
    
    // Server/Client data transfer errors
    SERVER_INVALID_RESULT_TYPE = -799,
    SERVER_ERROR_RECIEVING_RPC_ARGS = -800,
    SERVER_ERROR_SENDING_RPC_RESULT = -801,
    SERVER_ERROR_SENDING_RPC_ERRNO = -802,
    CLIENT_ERROR_SENDING_RPC_ARGS = -803,
    CLIENT_ERROR_RECIEVING_RPC_RESULT = -804,
    CLIENT_ERROR_RECIEVING_RPC_ERRNO = -805,
    
    // Checksum related errors
    CHECKSUM_BUFFER_ALLOC_ERROR = -900,
    CHECKSUM_BAD_READ_ERROR = -901,
} error_code;


#endif // ERROR_H
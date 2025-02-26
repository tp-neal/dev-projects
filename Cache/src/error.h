
#ifndef ERROR_H
#define ERROR_H

/***************************************************************************************************
* @project: Direct-Mapped Write-Back Cache [Trace Driven Simulation]
****************************************************************************************************
* @file cache.h
* @brief Contains macro and structure definitions, as well as function 
         declarations relating to error handling in the simulation.
*
* @author Tyler Neal
* @date 2/26/2025
***************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "config.h"

/*==================================================================================================
    Macros
==================================================================================================*/

// Macro for evaluating and handling an error
#define ERR_CHECK(err_info) do {                    \
    if (err_info.code != ERR_SUCCESS) {             \
        return handle_error(err_info);              \
    }                                               \
} while(0);

/*==================================================================================================
    Specific Error Data Structures
==================================================================================================*/

// Parameter-related error data
typedef struct {
    char* executable_name;
    unsigned int arg_count;
    unsigned int print_style;
} parameter_error_data_s;

// Cache-related error data
typedef struct {
    char type;
    unsigned int layer;
    unsigned int num_layers;
    size_t size;
    size_t line_size;
    size_t num_lines;
} cache_error_data_s;

// Request-related error data
typedef struct {
    char str_trace[TRACE_SIZE];
    char ref_type;
    char access_type;
    int index;
    size_t max_cache_index;
} request_error_data_s;

/*==================================================================================================
    General Error Structures
==================================================================================================*/

// Error domains
typedef enum {
    ERROR_NONE = 0,
    ERROR_PARAMETER,
    ERROR_CACHE,
    ERROR_REQUEST
} error_domain_s;

// Error codes
typedef enum {
    // Success
    ERR_SUCCESS = 0,

    // Failure
    ERR_FAILURE = -1,
    
    // Parameter errors (-100 to -199)
    ERR_INVALID_ARG_COUNT = -100,
    ERR_INVALID_CACHE_TYPE = -101,
    ERR_INVALID_LINE_SIZE = -102,
    ERR_INVALID_CACHE_LAYER_COUNT = -103,
    ERR_INVALID_CACHE_SIZE = -104,
    ERR_INVALID_PRINT_STYLE = -105,
    
    // Cache errors (-200 to -299)
    ERR_CACHE_ALLOCATION_FAILED = -200,
    ERR_CACHE_LINE_ALLOCATION_FAILED = -201,
    ERR_CACHE_IS_NULL = -202,
    ERR_CACHE_SIZE_NOT_POWER_OF_TWO = -203,
    
    // Request errors (-300 to -399)
    ERR_REQUEST_ALLOCATION_FAILED = -300,
    ERR_INVALID_REFERENCE_TYPE = -301,
    ERR_INVALID_ACCESS_TYPE = -302,
    ERR_REQUEST_IS_NULL = -303,
    ERR_REQUEST_ON_NULL_CACHE = -304,
    ERR_REQUEST_INDEX_OUT_OF_BOUNDS = -305,
    ERR_FAILED_TO_FORMAT_ADDRESS_HEX = -306,
} error_code_s;

// Unified error status structure
typedef struct {
    error_domain_s domain;
    error_code_s code;
    
    // Error-specific data
    union {
        parameter_error_data_s param;
        cache_error_data_s cache;
        request_error_data_s request;
    };
} error_status_s;

/*==================================================================================================
    Error Handler Function Declarations
==================================================================================================*/

int handle_error(error_status_s error);

int handle_param_error(error_status_s error);

int handle_cache_error(error_status_s error);

int handle_request_error(error_status_s error);


#endif // CACHE_H
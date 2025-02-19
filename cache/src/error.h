
#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>

/* ===================================================================
                              Macros
=================================================================== */
#define INSTRUCTION_SIZE 32

// Macro for evaluating and handling a request error
#define REQUEST_ERR_CHECK(err_info) do {            \
    if (err_info.error_code != REQUEST_SUCCESS) {   \
        return request_error_handler(err_info);     \
    }                                               \
} while(0)

// Macro for evaluating and handling a cache error
#define CACHE_ERR_CHECK(err_info) do {              \
    if (err_info.error_code != CACHE_SUCCESS) {     \
        return cache_error_handler(err_info);       \
    }                                               \
} while(0)

/* ===================================================================
                      Info Container Structures
=================================================================== */
typedef struct {
    char type;
    unsigned int layer;
    size_t num_layers;
    size_t line_size;
    size_t num_lines;
    size_t size;
} cache_info;

/* ===================================================================
                      Parameter Error Handling
=================================================================== */

typedef enum {
    PARAMETER_SUCCESS = 0,
    PARAMETER_INVALID_ARG_COUNT = -1,
    PARAMETER_INVALID_CACHE_TYPE = -2,
    PARAMETER_INVALID_LINE_SIZE = -3,
    PARAMETER_INVALID_CACHE_LAYER_COUNT = -4,
    PARAMETER_INVALID_CACHE_SIZE = -5,
    PARAMETER_INVALID_PRINT_STYLE = -6
} parameter_error_t;

typedef struct {
    parameter_error_t error_code;
    char* executable_name;
    unsigned int argument_count;
    cache_info cache;
    unsigned int print_style;
} parameter_status_t;

/* ===================================================================
                        Cache Error Handling
=================================================================== */

typedef enum {
    CACHE_SUCCESS = 0,
    CACHE_ALLOCATION_FAILED = -1,
    CACHE_LINE_ALLOCATION_FAILED = -2,
    CACHE_IS_NULL = -3,
    CACHE_SIZE_NOT_POWER_OF_TWO = -4
} cache_error_t;

typedef struct {
    cache_error_t error_code;
    cache_info cache;
} cache_status_t;

/* ===================================================================
                      Request Error Handling
=================================================================== */

typedef enum {
    REQUEST_SUCCESS = 0,
    REQUEST_ALLOCATION_FAILED = -1,
    REQUEST_INVALID_REFERENCE_TYPE = -2,
    REQUEST_INVALID_ACCESS_TYPE = -3,
    REQUEST_IS_NULL = -4,
    REQUEST_ON_NULL_CACHE = -5,
    REQUEST_INDEX_OUT_OF_BOUNDS = -6
} request_error_t;

typedef struct {
    request_error_t error_code;
    char ref_type;
    char access_type;
    unsigned int hex_address;
    char tag[INSTRUCTION_SIZE];
    char index[INSTRUCTION_SIZE];
    char offset[INSTRUCTION_SIZE];
} request_status_t;

/* ===================================================================
                      Error Handler Functions
=================================================================== */

/**
 * @brief Handles parameter errors and prints appropriate error message
 * 
 * @param error - parameter error information
 */
int parameter_error_handler(parameter_status_t error);

/**
 * @brief Handles cache errors and prints appropriate error message
 * 
 * @param error - cache error information
 */
int cache_error_handler(cache_status_t error);

/**
 * @brief Handles request errors and prints appropriate error message
 * 
 * @param error - request error information
 */
int request_error_handler(request_status_t error);

#endif // CACHE_H
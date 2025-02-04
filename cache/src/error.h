/*
===========================================================================
 PROJECT: Direct-Mapped Write-Back Cache [Trace Driven Simulation]
===========================================================================
 NAME : Tyler Neal
 USER ID : tpneal
 DATE: 01/09/2025
 FILE NAME : error.h
 DESCRIPTION:
    This header file declares error-related structuresm enums, and function
    declarations for the cache simulation.

 CONTENTS:
    - Parameter Error Types & Handling
    - Cache Error Types & Handling
    - Request Error Types & Handling
===========================================================================
*/

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>

/* ===================================================================
                      Parameter Error Handling
=================================================================== */

typedef enum {
    PARAMETER_SUCCESS = 0,
    PARAMETER_INVALID_ARG_COUNT,
    PARAMETER_INVALID_CACHE_TYPE,
    PARAMETER_INVALID_LINE_SIZE,
    PARAMETER_INVALID_CACHE_LAYER_COUNT,
    PARAMETER_INVALID_CACHE_SIZE,
    PARAMETER_INVALID_PRINT_STYLE
} parameter_error_t;

typedef struct {
    parameter_error_t error_code;
    char* executable_name;
    int argument_count;
    char cache_type;
    int line_size;
    int cache_layers;
    int cache_size;
    int layer;
    int print_style;
} parameter_error_info_t;

/* ===================================================================
                        Cache Error Handling
=================================================================== */

typedef enum {
    CACHE_SUCCESS = 0,
    CACHE_ALLOCATION_FAILED,
    CACHE_LINE_ALLOCATION_FAILED,
    CACHE_IS_NULL
} cache_error_t;

typedef struct {
    cache_error_t error_code;
    int layer;
    int cache_size;
    int line_size;
} cache_error_info_t;

/* ===================================================================
                      Request Error Handling
=================================================================== */

typedef enum {
    REQUEST_SUCCESS = 0,
    REQUEST_ALLOCATION_FAILED,
    REQUEST_INVALID_REFERENCE_TYPE,
    REQUEST_INVALID_ACCESS_TYPE,
    REQUEST_BINARY_CONVERSION_FAILED,
    REQUEST_IS_NULL,
    REQUEST_ON_NULL_CACHE
} request_error_t;

typedef struct {
    request_error_t error_code;
    char ref_type;
    char access_type;
    unsigned int address;
    char tag[21];        // Using MAX_TAG_SIZE
    char index[16];      // Using MAX_INDEX_SIZE
    char offset[5];      // Using MAX_OFFSET_SIZE
} request_error_info_t;

/* ===================================================================
                      Error Handler Functions
=================================================================== */

/**
 * @brief Handles parameter errors and prints appropriate error message
 * 
 * @param error - parameter error information
 */
int parameter_error_handler(parameter_error_info_t error);

/**
 * @brief Handles cache errors and prints appropriate error message
 * 
 * @param error - cache error information
 */
int cache_error_handler(cache_error_info_t error);

/**
 * @brief Handles request errors and prints appropriate error message
 * 
 * @param error - request error information
 */
int request_error_handler(request_error_info_t error);

#endif // CACHE_H
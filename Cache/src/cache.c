
/***************************************************************************************************
* @project: Direct-Mapped Write-Back Cache [Trace Driven Simulation]
****************************************************************************************************
* @file cache.c
* @brief Contains functions to simulate the cache by parsing trace files.
*
* This file contains the initialization logic for the various structures 
* required to model a direct-mapped write-back cache. This includes 
* structures for cache blocks, sets, and the complete cache.
*
*   The process is as follows:
*       1. Process relevant simulation information such as line_size, cache_size, 
*       ect.
*       2. Instantiate cache, as well as its set of lines.
*       3. Parse stdin for address references.
*       4. Decode address reference into request.
*       5. Process request as a read or write operation
*
* @author Tyler Neal
* @date 2/26/2025
***************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <time.h>
#include <math.h>

#include "cache.h"
#include "config.h"
#include "error.h"
 
/*==================================================================================================
    Main
==================================================================================================*/

/**
* @brief Main entry point for cache simulation program
* 
* @param argc Number of command-line arguments (expected 8)
* @param argv Array of command-line arguments:
*               - argv[0] Executable name
*               - argv[1] Cache type (U/I/D)
*               - argv[2] Line size in words
*               - argv[3] Number of cache layers (1-3)
*               - argv[4] L1 size in KB
*               - argv[5] L2 size in KB
*               - argv[6] L3 size in KB
*               - argv[7] Print style (1/2)
* @return int 0 on success, -1 on error
*/
int main(int argc, char* argv[]) {

    // Set up Timing Elements
    clock_t start_time, end_time;
    double elapsed_time;
    start_time = clock();

    // ========== Retreive Command-line Arguments ==========
    environment_info_s env;
    retrieveParameters(&env, argc, argv);

    // ========== Setup Cache Layers ==========
    error_status_s cache_setup_status;
    for (unsigned int i = 0; i < env.cache_layers; i++) {
        cache_setup_status = setupCache(
             &env.cache[i], 
             (i+1), 
        env.layer_sizes[i], 
         env.line_size);
        ERR_CHECK(cache_setup_status);
    }

    // ========== Process Reques ts Until End of File ==========
    char current_char;
    char buffer[TRACE_SIZE+1]; // traces are 11 char's long + null terminator
    buffer[TRACE_SIZE] = '\0';
    bool data_found = false;

    // Initialize request
    request_s* request;
    error_status_s request_status = allocateRequest(&request);
    ERR_CHECK(request_status);

    // Process each trace in the input file
    while ((current_char = getchar()) != EOF) {
        // Skip until trace found
        if (current_char != '@') continue;

        // Trace found, read in format: <I/D><R/W><hex-address>
        fgets(buffer, sizeof(buffer), stdin);

        // Skip request for irrelevant cache types
        if (env.cache_type != UNIFIED && 
           (reference_type_e)buffer[0] != env.cache_type) {
            continue;
        }

        // Try each cache layer in order until data is found
        for (unsigned int i = 0; i < env.cache_layers; i++) {
            env.cache[i]->requests++;

            // Format the request information for cache layer
            request_status = formatRequest(request, env.cache[i], buffer);
            ERR_CHECK(request_status);

            // Process request in given cache layer
            request_status = processRequest(request, env.cache[i], &data_found);
            ERR_CHECK(request_status);

            if (data_found) break; // exit loop on cache hit
        }

        // Reset flag
        data_found = false;
    }

    // ========== Print Statistics and Cleanup ==========
    printResults(env);

    // Clean up memory
    free(request);
    for (unsigned int i = 0; i < env.cache_layers; i++) {
        destroyCache(env.cache[i]);
    }

    // Calculate and print elapsed time
    end_time = clock();
    elapsed_time = (((double)(end_time - start_time)) / CLOCKS_PER_SEC);
    printf("Total Elapsed Time: %.2f seconds\n", elapsed_time);

    // End of program
    return EXIT_SUCCESS;
}

/*==================================================================================================
    Parameter Handling
==================================================================================================*/

/**
 * @brief Retrieves and validates command-line parameters for the cache simulation
 * 
 * @param env Pointer to environment_info_s structure to store the parameters
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return error_status_s Error status structure with any parameter errors
 */
error_status_s retrieveParameters(environment_info_s* env, int argc, char** argv) {
    error_status_s param_status = {  // holds error info
        .domain = ERROR_PARAMETER,
        .param.executable_name = argv[0]
    };

    // Check argument count
    if (argc != 8) {
        param_status.code = ERR_INVALID_ARG_COUNT;
        param_status.param.arg_count = argc;
        return param_status;
    }

    // Get type of cache reference to be tracked
    env->cache_type = (reference_type_e) *argv[1]; // cast the character
    if (env->cache_type != UNIFIED && 
        env->cache_type != INSTRUCTION && 
        env->cache_type != DATA) 
    {   
        param_status.code = ERR_INVALID_CACHE_TYPE;
        param_status.cache.type = env->cache_type;
        return param_status;
    }
    
    // Get size of each line in all caches
    env->line_size = (atoi(argv[2]) * 4); // convert words to bytes
    if (env->line_size < 4) { // cant be smaller than 4 bytes
        param_status.code = ERR_INVALID_LINE_SIZE;
        param_status.cache.line_size = env->line_size;
        return param_status;
    }

    // Get total number of cache layers
    env->cache_layers = atoi(argv[3]);
    if (env->cache_layers < 1 || env->cache_layers > 3) { // we only support (1-3) layers
        param_status.code = ERR_INVALID_CACHE_LAYER_COUNT;
        param_status.cache.num_layers = env->cache_layers;
        return param_status;
    }
    
    // Get cache sizes (0 indicates nonexistent cache layer)
    for (unsigned int i = 0; i < env->cache_layers; i++)
        env->layer_sizes[i] = (atoi(argv[i+4]) * 1024);

    // Validate integrity of computed cache sizes
    for (unsigned int i = 0; i < env->cache_layers; i++) {
        // Make sure cache is valid size and lines are properly addressable
        if (env->layer_sizes[i] % env->line_size != 0) {
            param_status.code = ERR_INVALID_CACHE_SIZE;
            param_status.cache.size = env->layer_sizes[i];
            param_status.cache.layer = (i+1); // increment b/c we're 0-indexed
            return param_status;
        }
    }

    // Get print style
    env->print_style = atoi(argv[7]); // 1 concise : 2 verbose
    if (env->print_style != 1 && env->print_style != 2) {
        param_status.code = ERR_INVALID_PRINT_STYLE;
        param_status.param.print_style = env->print_style;
        return param_status;
    }

    return param_status;
}

/*==================================================================================================
    Cache Functions
==================================================================================================*/

/**
 * @brief Allocates memory for a cache structure
 * 
 * @param cache Double pointer to cache instance to allocate
 * @param layer Cache layer number (1-3)
 * @param cache_size Total size of cache in bytes
 * @param line_size Size of each cache line in bytes
 * @return error_status_s Status structure with error code and cache metadata
 */
error_status_s allocateCache(cache_s** cache, size_t layer, size_t cache_size, size_t line_size) {
    error_status_s status = {
        .domain = ERROR_CACHE,
        .code = ERR_SUCCESS,
        .cache.layer = layer,
        .cache.size = cache_size,
        .cache.line_size = line_size
    };

    // Allocate space for cache
    *cache = (cache_s*)malloc(sizeof(cache_s));
    if (*cache == NULL) {
        status.code = ERR_CACHE_ALLOCATION_FAILED;
    }

    return status;
}

/**
 * @brief Frees memory allocated for a cache structure
 * 
 * @param cache Pointer to the cache structure to destroy
 */
void destroyCache(cache_s* cache) {
    // Check for null pointer
    if (cache != NULL) {
        free(cache->lines);
        free(cache);
    }

    if (DEBUG) {
        printf("Cache successfully deleted\n");
    }
}

/**
 * @brief Allocates and initializes a cache structure with given parameters
 * 
 * @param cache Double pointer to cache structure to initialize
 * @param layer Cache layer number (1-3)
 * @param cache_size Total size of cache in bytes
 * @param line_size Size of each cache line in bytes
 * @return error_status_s Error status structure indicating success or failure
 */
error_status_s setupCache(cache_s** cache, size_t layer, size_t cache_size, size_t line_size) {

    // Allocate space for cache on the heap
    error_status_s status = allocateCache(cache, layer, cache_size, line_size);
    if (status.code != ERR_SUCCESS) {
        return status;
    }

    // Initialize cache struct values
    (*cache)->layer = layer;
    (*cache)->cache_size = cache_size;
    (*cache)->line_size = line_size;
    (*cache)->num_lines = (cache_size / line_size);
    (*cache)->requests = 0;
    (*cache)->hits = 0;
    (*cache)->misses = 0;
    (*cache)->read_to_write = 0;
    (*cache)->write_to_write = 0;

    // Verify number of lines is a power of two
    if (!isPowerOfTwo((*cache)->num_lines)) {
        status.code = ERR_CACHE_SIZE_NOT_POWER_OF_TWO;
        status.cache.num_lines = (*cache)->num_lines;
        return status;
    }

    // Memory allocation for cache lines
    (*cache)->lines = (line_s*)malloc((*cache)->num_lines * sizeof(line_s));
    if ((*cache)->lines == NULL) {
        status.code = ERR_CACHE_LINE_ALLOCATION_FAILED;
        return status;
    }

    // Default each line's attributes
    for (size_t i = 0; i < (*cache)->num_lines; i++) {
        (*cache)->lines[i].dirty = false; // marks line as initially clean (not modified)
        (*cache)->lines[i].tag[0] = '\0';  // initial tag set to '\0' to represent uninitialized
    }

    // Calculate Address Field Sizes
    (*cache)->offset_size = (size_t)(log2(line_size));
    (*cache)->index_size = (size_t)(log2((int)(cache_size / line_size)));
    (*cache)->tag_size = (size_t)(INSTRUCTION_SIZE - 
                                 (*cache)->index_size - 
                                 (*cache)->offset_size);

    if (DEBUG) {
        printf( "\n"
                "Cache Created:\n"
                "---------------\n"
                "Layer: %zu\n"
                "Size (bytes): %zu\n"
                "Line Size (bytes): %zu\n"
                "Number of Lines: %zu\n"
                "Tag Size: %d\n"
                "Index Size: %d\n"
                "Offset Size: %d\n",
                layer, cache_size, line_size, (*cache)->num_lines, 
                (*cache)->tag_size, (*cache)->index_size, (*cache)->offset_size);
    }

    return status;
}

/*==================================================================================================
    Request Functions
==================================================================================================*/

/**
 * @brief Allocates memory for a request structure
 * 
 * @param request Double pointer to request structure to allocate
 * @return error_status_s Error status structure indicating success or failure
 */
error_status_s allocateRequest(request_s** request) {
    error_status_s status = {
        .domain = ERROR_REQUEST,
        .code = ERR_SUCCESS
    };

    // Allocate memory for Request
    *request = (request_s*)malloc(sizeof(request_s));
    if (*request == NULL) {
        status.code = ERR_REQUEST_ALLOCATION_FAILED;
    }

    return status;
}

/**
 * @brief Formats a request structure based on a trace buffer and cache parameters
 * 
 * @param request Pointer to request structure to format
 * @param cache Pointer to the cache structure for address field sizing
 * @param buffer Input trace buffer string containing request information
 * @return error_status_s Error status structure indicating success or failure
 */
error_status_s formatRequest(request_s* request, cache_s* cache, const char* buffer) {
    error_status_s status = {
        .domain = ERROR_REQUEST,
        .code = ERR_SUCCESS
    };

    if (request == NULL) {
        status.code = ERR_REQUEST_IS_NULL;
        return status;
    }

    if (cache == NULL) {
        status.code = ERR_CACHE_IS_NULL;
        return status;
    }

    // Assign address field sizes based on cache
    request->address.tag_size = cache->tag_size;
    request->address.index_size = cache->index_size;
    request->address.offset_size = cache->offset_size;

    // Note: Buffer format: <I/D><R/W><hex-address>
    // Assign reference type based on trace
    switch (buffer[0]) {
        case 'I':
            request->ref_type = INSTRUCTION;
            break;
        case 'D':
            request->ref_type = DATA;
            break;
        default:
            status.code = ERR_INVALID_REFERENCE_TYPE;
            status.request.ref_type = buffer[0];
            return status;
    }

    // Assign access type based on trace
    switch (buffer[1]) {
        case 'R':
            request->access_type = READ;
            break;
        case 'W':
            request->access_type = WRITE;
            break;
        default:
            status.code = ERR_INVALID_ACCESS_TYPE;
            status.request.access_type = buffer[1];
            return status;
    }

    // Format the address fields of the request
    status = formatRequestAddressFields(request, buffer);
    if (status.code != ERR_SUCCESS) 
        return status;

    if (DEBUG) {
        printf( "\nRequest Formatted:\n"
                  "-------------------\n"
                  "Reference Type: %c\n"
                  "Access Type: %c\n"
                  "Cache Layer: %zu\n"
                  "Hex Address: %x\n"
                  "Binary Address: %s\n"
                  "Tag Bits: %s\n"
                  "Tag Dec: %u\n"
                  "Index Bits: %s\n"
                  "Index Dec: %u\n"
                  "Offset Bits: %s\n"
                  "Offset Dec: %u\n",
                  request->ref_type, request->access_type,
                  cache->layer, request->address.hex, request->address.binary,
                  request->address.tag, binaryStringToInt(request->address.tag),
                  request->address.index, binaryStringToInt(request->address.index),
                  request->address.offset, binaryStringToInt(request->address.offset));
    }

    return status;
}

/**
 * @brief Formats the address fields of a request from a trace buffer
 * 
 * @param buffer Input trace buffer string containing address information
 * @param request Pointer to request structure to populate with address fields
 * @return error_status_s Error status structure indicating success or failure
 */
error_status_s formatRequestAddressFields(request_s* request, const char* buffer) {
    error_status_s status = {
        .domain = ERROR_REQUEST,
        .code = ERR_SUCCESS
    };

    // Format hex address and ensure it's proper
    if (sscanf(buffer + 2, "%x", &request->address.hex) != 1) {
        status.code = ERR_FAILED_TO_FORMAT_ADDRESS_HEX;
        memcpy(&status.request.str_trace, buffer, TRACE_SIZE);
        return status;
    }

    // Convert address to binary representation
    hexToBinaryString(request->address.binary, request->address.hex);
    
    // Retrieve and copy tag, index, and offset
    int tag_size = request->address.tag_size;
    int index_size = request->address.index_size;
    int offset_size = request->address.offset_size;
    strncpy(
       request->address.tag, 
        request->address.binary, // start at index 0
          tag_size
    );
    request->address.tag[tag_size] = '\0'; // end with null term

    strncpy(
       request->address.index, 
        request->address.binary + tag_size, // skip tag
          index_size
    );
    request->address.index[index_size] = '\0'; // end with null term

    strncpy(
      request->address.offset, 
       request->address.binary + tag_size + index_size, // skip tag + index
         offset_size
    );
    request->address.offset[offset_size] = '\0'; // end with null term

    return status;
}

/**
 * @brief Processes a cache request, updating cache state and statistics
 * 
 * @param request Pointer to formatted request to process
 * @param cache Pointer to cache structure to access
 * @param hit_occured Pointer to boolean indicating if a cache hit occurred
 * @return error_status_s Error status structure indicating success or failure
 */
error_status_s processRequest(request_s* request, cache_s* cache, bool* hit_occured) {
    error_status_s status = {
        .domain = ERROR_REQUEST,
        .code = ERR_SUCCESS
    };

    // Convert index to integer
    size_t index = binaryStringToInt(request->address.index);

    // Check if index is within bounds
    if (index >= cache->num_lines) {
        status.code = ERR_REQUEST_INDEX_OUT_OF_BOUNDS;
        return status;
    }

    // Create reference to line for clarity
    line_s* line = &cache->lines[index];
    access_type_e acc_type = request->access_type; // store access_type for clarity

    // If line found in cache
    if (strcmp(line->tag, request->address.tag) == 0) {
        cache->hits++;
        *hit_occured = true;
        if (acc_type == 'W') 
            line->dirty = true;  // data is now modified

    } else {
        cache->misses++;
        *hit_occured = false;
        if (line->dirty == true) {
            if (acc_type == 'R') cache->read_to_write++;
            if (acc_type == 'W') cache->write_to_write++;
        }

        // Load the new tag, mark as clean
        strcpy(line->tag, request->address.tag);
        if (acc_type == 'R') line->dirty = false;
        if (acc_type == 'W') line->dirty = true;
    }

    return status;
}

/*==================================================================================================
    Error Handling
==================================================================================================*/

/**
 * @brief Top-level error handler that delegates to domain-specific handlers
 * 
 * @param error Error status structure containing error information
 * @return int ERR_FAILURE after displaying error message
 */
int handle_error(error_status_s error) {
    fprintf(stderr, "Error: ");

    switch(error.domain) {

        default:
            fprintf(stderr, "Invalid error domain: %d\n", error.domain);
            break;

        case ERROR_NONE:
            fprintf(stderr, "Error must have domain\n");
            break;
        
        case ERROR_PARAMETER:
            return handle_param_error(error);

        case ERROR_CACHE:
            return handle_cache_error(error);

        case ERROR_REQUEST:
            return handle_request_error(error);
    }
    return ERR_FAILURE;
}

/**
 * @brief Handles parameter-related errors with appropriate error messages
 * 
 * @param error Error status structure containing parameter error information
 * @return int ERR_FAILURE after displaying error message
 */
int handle_param_error(error_status_s error) {
    
    switch(error.code) {

        /** GENERAL CASES **/
        default:
            fprintf(stderr, "Invalid error code: %d\n", error.code);
            break;

        case ERR_SUCCESS:
            fprintf(stderr, "ERR_SUCCESS unintentionally passed to handler\n");
            break;

        /** PARAMETER ERRORS **/
        case ERR_INVALID_ARG_COUNT:
            fprintf(stderr, "Invalid number of arguments. "
                    "Expected 8, received %d.\n"
                    "Usage: %s <cache_type> <line_size> <cache_layers>" 
                    "<L1_size_B> <L2_size_B> <L3_size_B> <print_style>\n",
                    error.param.arg_count, error.param.executable_name);
            break;
        case ERR_INVALID_CACHE_TYPE:
            fprintf(stderr, 
                    "Invalid cache type '%c'.\n", 
                    error.cache.type);
            break;
        case ERR_INVALID_LINE_SIZE:
            fprintf(stderr, 
                    "Invalid line size '%zu'.\n", 
                    error.cache.line_size);
            break;
        case ERR_INVALID_CACHE_LAYER_COUNT:
            fprintf(stderr, 
                    "Invalid cache layer count '%hu'.\n", 
                    error.cache.num_layers);
            break;
        case ERR_INVALID_CACHE_SIZE:
            fprintf(stderr, 
                    "Invalid cache size '%zu' for layer '%u'.\n", 
                    error.cache.size, error.cache.layer);
            break;
        case ERR_INVALID_PRINT_STYLE:
            fprintf(stderr, 
                    "Invalid print style '%u'.\n"
                    "Usage: 1 = standard print | 2 = debug print\n", 
                    error.param.print_style);
            break;
    }
    return ERR_FAILURE;
}

/**
 * @brief Handles cache-related errors with appropriate error messages
 * 
 * @param error Error status structure containing cache error information
 * @return int ERR_FAILURE after displaying error message
 */
int handle_cache_error(error_status_s error) {

    switch (error.code) {

        /** GENERAL CASES **/
        default:
            fprintf(stderr, "Invalid error code: %d\n", error.code);
            break;

        case ERR_SUCCESS:
            fprintf(stderr, "ERR_SUCCESS unintentionally passed to handler\n");
            break;

        /** CACHE ERRORS **/
        case ERR_CACHE_ALLOCATION_FAILED:
            fprintf(stderr, 
                    "Failed to allocate cache "
                    "{ layer:%d | size:%lu | line_size:%lu }\n",
                    error.cache.layer, error.cache.size, error.cache.line_size);
            break;
        case ERR_CACHE_LINE_ALLOCATION_FAILED:
            fprintf(stderr, 
                    "Failed to allocate cache layer:%d\n", 
                    error.cache.layer);
            break;
        case ERR_CACHE_IS_NULL:
            fprintf(stderr, "Cache is null\n");
            break;
        case ERR_CACHE_SIZE_NOT_POWER_OF_TWO:
            fprintf(stderr, "Cache size is not power of two, "
                    "size:%lu\n",
                    error.cache.size);
            break;
    }
    return ERR_FAILURE;
}

/**
 * @brief Handles request-related errors with appropriate error messages
 * 
 * @param error Error status structure containing request error information
 * @return int ERR_FAILURE after displaying error message
 */
int handle_request_error(error_status_s error) {

    switch(error.code) {

        /** GENERAL CASES **/
        default:
            fprintf(stderr, "Invalid error code: %d\n", error.code);
            break;

        case ERR_SUCCESS:
            fprintf(stderr, "ERR_SUCCESS unintentionally passed to handler\n");
            break;

        /** REQUEST ERRORS **/
        case ERR_REQUEST_ALLOCATION_FAILED:
            fprintf(stderr, "Failed to allocate request\n");
            break;
        case ERR_INVALID_REFERENCE_TYPE:
            fprintf(stderr, "Reference type of \"%c\" is not valid\n",
                    error.request.ref_type);
            break;
        case ERR_INVALID_ACCESS_TYPE:
            fprintf(stderr, "Access type of \"%c\" is not valid\n",
                    error.request.access_type);
            break;
        case ERR_REQUEST_IS_NULL:
            fprintf(stderr, "Request is null\n");
            break;
        case ERR_REQUEST_ON_NULL_CACHE:
            fprintf(stderr, "Requested cache is null\n");
            break;
        case ERR_REQUEST_INDEX_OUT_OF_BOUNDS:
            fprintf(stderr, "Requested index out of bounds. (0-Indexed) Index: "
                    "%d, Number_of_Lines: %ld\n",
                    error.request.index, error.request.max_cache_index);
            break;
        case ERR_FAILED_TO_FORMAT_ADDRESS_HEX:
            fprintf(stderr, "Failed to format hex address for trace %s\n",
                    error.request.str_trace);
            break;
    }
    return ERR_FAILURE;
}

/*==================================================================================================
    Print Functions
==================================================================================================*/

/**
 * @brief Prints simulation results for all cache layers
 * 
 * @param env Environment information structure containing cache configurations
 * @return error_status_s Error status structure indicating success or failure
 */
error_status_s printResults(environment_info_s env) {
    error_status_s status;

    // Print individual layer statistics
    for (unsigned int i = 0; i < env.cache_layers; i++) {
        status = printCacheStats(env.cache[i], env.print_style);
        if (status.code != ERR_SUCCESS)
            return status;
    }

    // Print average memory access times;
    printAMAT(env.cache, env.cache_layers);

    return status;
}

/**
 * @brief Prints statistics for a single cache layer
 * 
 * @param cache Pointer to cache structure containing statistics
 * @param style Print style (1 for concise, 2 for verbose)
 * @return error_status_s Error status structure indicating success or failure
 */
error_status_s printCacheStats(cache_s* cache, unsigned int print_style) {
    error_status_s status = {
        .code = ERR_SUCCESS
    };
    
    // Check for null pointer
    if (cache == NULL) {
        status.code = ERR_CACHE_IS_NULL;
        return status;
    }

    // Concise Print - used for "Part 1" (run1.sh)
    if (print_style == 1) {
        printf("Total Requests: %zu\n", cache->requests);
        printf("     Miss Rate: %.2f%%\n", ((float)cache->misses / (float)cache->requests) * 100);
        printf("------------------------------------------------------------\n");

    // Verbose Print - used for "Part 2" (run2.sh)
    } else if (print_style == 2) {
        printf("\nCache Layer: %zu\n", cache->layer);
        printf("----------------\n");
        printf("Configuration:\n");
        printf("    Size: %zu bytes\n", cache->cache_size);
        printf("    Line Size: %zu bytes\n", cache->line_size);
        printf("    Line Count: %zu\n", cache->num_lines);
        printf("Performance Metrics:\n");
        printf("    Total Requests: %zu\n", cache->requests);
        printf("    Hits: %zu\n", cache->hits);
        printf("    Misses: %zu\n", cache->misses);
        printf("    Hit Rate: %.2f%%\n", ((float)cache->hits / (float)cache->requests) * 100);
        printf("    Miss Rate: %.2f%%\n", ((float)cache->misses / (float)cache->requests) * 100);
        printf("    Read to Write Ratio: %zu\n", cache->read_to_write);
        printf("    Write to Write Ratio: %zu\n", cache->write_to_write);
    }

    return status;
}

/**
 * @brief Prints Average Memory Access Time (AMAT) for the cache hierarchy
 * 
 * @param cache Array of pointers to cache structures
 * @param layers Number of cache layers
 * @param print_style Print style (1 for concise, 2 for verbose)
 */
void printAMAT(cache_s** cache, size_t layers) {

    // Calculate miss rates
    float miss_rates[3];
    for (unsigned int i = 0; i < layers; i++) {
        miss_rates[i] = ((float)cache[i]->misses / (float)cache[i]->requests)   ;
    }

    // Calculate and display average memory access time for each layer
    printf("------------------------------------------------------------\n");
    if (layers == 1) {
        printf("AMAT: %.2f\n", 
               (HIT_TIME_L1 + 
               (miss_rates[0] * MEM_ACCESS_TIME)));

    } else if (layers == 2) {
        printf("AMAT: %.2f\n", 
               (HIT_TIME_L1 + 
               (miss_rates[0] * (HIT_TIME_L2 +
               (miss_rates[1] * MEM_ACCESS_TIME)))));

    } else if (layers == 3) {
        printf("AMAT: %.2f\n", 
               (HIT_TIME_L1 +
               (miss_rates[0] * (HIT_TIME_L2 +
               (miss_rates[1] * HIT_TIME_L3 +
               (miss_rates[2] * MEM_ACCESS_TIME))))));
    }
    printf("------------------------------------------------------------\n");
}

/*==================================================================================================
    Additional Helpers
==================================================================================================*/

/**
 * @brief Converts a hexadecimal integer to a binary string representation
 * 
 * @param binary Output buffer for binary string (must be at least 33 bytes)
 * @param hex Input hexadecimal integer to convert
 */
void hexToBinaryString(char* binary, unsigned int hex) {
    for (int i = 31; i >= 0; i--) {
        binary[31 - i] = ((hex >> i) & 1) ? '1' : '0';
    }
    binary[32] = '\0';
}

/**
 * @brief Converts a binary string representation to an unsigned integer
 * 
 * @param binary Input binary string to convert
 * @return unsigned int Converted integer value
 */
unsigned int binaryStringToInt(const char* binary) {
    unsigned int result = 0;
    for (int i = 0; binary[i] != '\0' && i < 32; i++) {
        result <<= 1;
        if (binary[i] == '1') {
            result |= 1;
        }
    }
    return result;
}

/**
 * @brief Checks if a number is a power of two
 * 
 * @param n Number to check
 * @return bool True if n is a power of two, false otherwise
 */
bool isPowerOfTwo(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

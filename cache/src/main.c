
/*
===========================================================================
 PROJECT: Direct-Mapped Write-Back Cache [Trace Driven Simulation]
===========================================================================
 NAME: Tyler Neal
 DATE: 02/18/2025
 FILE NAME: main.c
 DESCRIPTION:
    This file contains the initialization logic for the various structures 
    required to model a direct-mapped write-back cache. This includes 
    structures for cache blocks, sets, and the complete cache.

 PSEUDO :
    1. Process relevant simulation information such as line_size, cache_size, 
       ect.
    2. Instantiate cache, as well as its set of lines.
    3. Parse stdin for address references.
    4. Decode address reference into request.
    5. Process request as a read or write operation
===========================================================================
*/

// Inclusions
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "cache.h"
#include "error.h"


// Global Variables
unsigned int cache_layers;
 
// ========================================================================
//                               Main
// ========================================================================

/**
* @brief Main entry point for cache simulation program
* 
* @param argc Number of command-line arguments (expected 8)
* @param argv Array of command-line arguments:
*             [0] Executable name
*             [1] Cache type (U/I/D)
*             [2] Line size in words
*             [3] Number of cache layers (1-3)
*             [4] L1 size in KB
*             [5] L2 size in KB
*             [6] L3 size in KB
*             [7] Print style (1/2)
* @return int EXIT_SUCCESS on success, EXIT_FAILURE on error
*/
int main(int argc, char* argv[]) {

    // ========== Set up Timing Elements ==========
    clock_t start_time, end_time;
    double elapsed_time;
    start_time = clock();


    // ========== Verify Command-line Arguments ==========
    parameter_status_t parameter_status = {  // holds error info
        .error_code = PARAMETER_SUCCESS,
        .executable_name = argv[0]
    };

    // Check argument count
    if (argc != 8) {
        parameter_status.error_code = PARAMETER_INVALID_ARG_COUNT;
        parameter_status.argument_count = argc;
        return parameter_error_handler(parameter_status);
    }

    // Get type of cache reference to be tracked
    Cache_Type cache_type = (Cache_Type) *argv[1]; // cast the character
    if (cache_type != UNIFIED && cache_type != INSTRUCTION && cache_type != DATA) {
        parameter_status.error_code = PARAMETER_INVALID_CACHE_TYPE;
        parameter_status.cache.type = cache_type;
        return parameter_error_handler(parameter_status);
    }
    
    // Get size of each line in all caches
    size_t line_size = (atoi(argv[2]) * 4);  // line size parameter is in unit of words. We mutliply by 4 to get bytes
    
    // || line_size > 64 (took cond out temporarily)
    if (line_size < 4) { // cannot physically be smaller than 4 bytes, or larger than 64 bytes in our example
        parameter_status.error_code = PARAMETER_INVALID_LINE_SIZE;
        parameter_status.cache.line_size = line_size;
        return parameter_error_handler(parameter_status);
    }

    // Get total number of cache layers
    cache_layers = atoi(argv[3]);
    if (cache_layers < 1 || cache_layers > 3) { // we only support a 1-3 layer cache system here
        parameter_status.error_code = PARAMETER_INVALID_CACHE_LAYER_COUNT;
        parameter_status.cache.num_layers = cache_layers;
        return parameter_error_handler(parameter_status);
    }
    
    // Get cache sizes (0 indicates nonexistent cache layer)
    size_t L1_size_B = (atoi(argv[4]) * 1024); // cache sizes are in unit of KB. We multiply by 1024 to convert to bytes
    size_t L2_size_B = (atoi(argv[5]) * 1024);
    size_t L3_size_B = (atoi(argv[6]) * 1024); 
    size_t cache_sizes[] = {L1_size_B, L2_size_B, L3_size_B};

    // Validate integrity of computed cache sizes
    for (unsigned int i = 0; i < cache_layers; i++) {

        // Make sure cache is valid size and lines are properly addressable
        if (cache_sizes[i] % line_size != 0) {
            parameter_status.error_code = PARAMETER_INVALID_CACHE_SIZE;
            parameter_status.cache.size = cache_sizes[i];
            parameter_status.cache.layer = (i+1); // increment since we are 0-indexed
            return parameter_error_handler(parameter_status);
        }
    }

    // Get print style
    unsigned int print_style = atoi(argv[7]); // style of 1 prints total request and miss rate, while style 2 prints more detailed view
    if (print_style != 1 && print_style != 2) {
        parameter_status.error_code = PARAMETER_INVALID_PRINT_STYLE;
        parameter_status.print_style = print_style;
        return parameter_error_handler(parameter_status);
    }


    // Allocate and initialize cache layers
    Cache* cache[3]; // each index represents a cache layer where layer number = index + 1
    size_t layer_sizes[3] = {L1_size_B, L2_size_B, L3_size_B};
    cache_status_t cache_setup_status;
    for (unsigned int i = 0; i < cache_layers; i++) {
        cache_setup_status = setupCache(&cache[i], (i+1), layer_sizes[i], line_size);
        CACHE_ERR_CHECK(cache_setup_status);
    }


    // ========== Process Requests Until End of File ==========
    char current_char;
    char buffer[12]; // traces are 11 char's long + null terminator
    buffer[11] = '\0';
    bool data_found = false;

    // Initialize request
    Request* request;
    request_status_t request_status = allocateRequest(&request);
    REQUEST_ERR_CHECK(request_status);

    // Process each trace in the input file
    while ((current_char = getchar()) != EOF) {
        if (current_char != '@') continue; // skip until trace found

        // Trace found, read in format: <I/D><R/W><hex-address>
        fgets(buffer, sizeof(buffer), stdin);

        // Try each cache layer in order until data is found
        for (unsigned int i = 0; i < cache_layers; i++) {

            // Format the request information for cache layer
            request_status = formatRequest(request, cache[i], buffer);
            REQUEST_ERR_CHECK(request_status);

            // If request type is not that of cache, skip request
            if (request->ref_type != cache_type && cache_type != UNIFIED) {
                break;
            }

            // Process request in given cache layer
            cache[i]->requests++;
            request_status = processRequest(request, cache[i], &data_found);
            REQUEST_ERR_CHECK(request_status);
            if (data_found) break; // exit loop on cache hit
        }

        // Reset flag
        data_found = false;
    }


    // ========== Calculate & Prints Cache Statistics ==========
    // Calculate miss rates
    float miss_rates[3];
    for (unsigned int i = 0; i < cache_layers; i++) {
        miss_rates[i] = ((float)cache[i]->misses / (float)cache[i]->requests)   ;
    }

    // Print cache statistics
    cache_status_t print_status;
    for (unsigned int i = 0; i < cache_layers; i++) {
        print_status = printCacheStats(cache[i], print_style);
        CACHE_ERR_CHECK(print_status);
    }

    // Calculate and display average memory access time for each layer
    printf("------------------------------------------------------------\n");
    if (cache_layers == 1) {
        printf("AMAT: %.2f\n", (HIT_TIME_L1+(miss_rates[0]*MEM_ACCESS_TIME)));
    } else if (cache_layers == 2) {
        printf("AMAT: %.2f\n", (HIT_TIME_L1+(miss_rates[0]*(HIT_TIME_L2+(miss_rates[1]*MEM_ACCESS_TIME)))));
    } else if (cache_layers == 3) {
        printf("AMAT: %.2f\n", (HIT_TIME_L1+(miss_rates[0]*(HIT_TIME_L2+(miss_rates[1]*HIT_TIME_L3+(miss_rates[2]*MEM_ACCESS_TIME))))));
    }
    printf("------------------------------------------------------------\n");

    // Clean up memory
    destroyRequest(request);
    for (unsigned int i = 0; i < cache_layers; i++) {
        destroyCache(cache[i]);
    }

    // Calculate and print elapsed time
    end_time = clock();
    elapsed_time = (((double)(end_time - start_time)) / CLOCKS_PER_SEC);
    printf("Total Elapsed Time: %.2f seconds\n", elapsed_time);

    // End of program
    return EXIT_SUCCESS;
}


//========================================================================
//                          Cache Functions 
//========================================================================

/**
 * @brief Allocates memory for a cache structure
 * 
 * @param cache Double pointer to cache instance to allocate
 * @param layer Cache layer number (1-3)
 * @param cache_size Total size of cache in bytes
 * @param line_size Size of each cache line in bytes
 * @return cache_status_t Status structure with error code and cache metadata
 */
cache_status_t allocateCache(Cache** cache, unsigned int layer, size_t cache_size, size_t line_size) {
    cache_status_t cache_status = {
        .error_code = CACHE_SUCCESS,
        .cache.layer = layer,
        .cache.size = cache_size,
        .cache.line_size = line_size
    };

    // Allocate space for cache
    *cache = (Cache*)malloc(sizeof(Cache));
    if (*cache == NULL) {
        cache_status.error_code = CACHE_ALLOCATION_FAILED;
    }

    return cache_status;
}

/**
 * @brief Deallocates memory for a cache given its pointer
 * 
 * @param cache Pointer to cache to be destroyed
 */
void destroyCache(Cache* cache) {
    // Check for null pointer
    if (cache != NULL) {
        if (cache->lines != NULL) {
            free(cache->lines);
        }
        free(cache);
    }

    // Debug Print
    if (DEBUG) {
        printf("Cache successfully deleted\n");
    }
}

/**
 * @brief Initializes cache structure and calculates address field sizes
 * 
 * @param cache Double pointer to cache instance to initialize
 * @param layer Cache layer number (1-3)
 * @param cache_size Total size of cache in bytes
 * @param line_size Size of each cache line in bytes
 * @return cache_status_t Status structure with error code and cache metadata
 */
cache_status_t setupCache(Cache** cache, unsigned int layer, size_t cache_size, size_t line_size) {

    // Allocate space for cache on the heap
    cache_status_t cache_status = allocateCache(cache, layer, cache_size, line_size);
    if (cache_status.error_code != CACHE_SUCCESS) {
        return cache_status;
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
        cache_status.error_code = CACHE_SIZE_NOT_POWER_OF_TWO;
        cache_status.cache.num_lines = (*cache)->num_lines;
        return cache_status;
    }

    // Memory allocation for cache lines
    (*cache)->lines = (Line*)malloc((*cache)->num_lines * sizeof(Line));
    if ((*cache)->lines == NULL) {
        cache_status.error_code = CACHE_LINE_ALLOCATION_FAILED;
        return cache_status;
    }

    // Default each line's attributes
    for (size_t i = 0; i < (*cache)->num_lines; i++) {
        (*cache)->lines[i].dirty = false; // marks line as initially clean (not modified)
        (*cache)->lines[i].tag[0] = 'x';  // initial tag set to 'x' to represent uninitialized
    }

    // Calculate Address Field Sizes
    (*cache)->offset_size = (size_t)(log2(line_size));
    (*cache)->index_size = (size_t)(log2(cache_size / line_size));
    (*cache)->tag_size = (size_t)(INSTRUCTION_SIZE - (*cache)->index_size - (*cache)->offset_size);

    // Debug Print
    if (DEBUG) {
        printf( "\nCache Created:\n"
                  "---------------\n"
                  "Layer: %u\n"
                  "Size (bytes): %zu\n"
                  "Line Size (bytes): %zu\n"
                  "Number of Lines: %zu\n"
                  "Tag Size: %d\n"
                  "Index Size: %d\n"
                  "Offset Size: %d\n",
                  layer, cache_size, line_size, (*cache)->num_lines, 
                  (*cache)->tag_size, (*cache)->index_size, (*cache)->offset_size);
    }

    return cache_status;
}

/**
 * @brief Prints cache statistics and configuration details
 * 
 * @param cache Pointer to cache instance to display stats for
 * @param style Output style (1 = compact, 2 = verbose)
 * @return cache_status_t Status structure with error code
 */
cache_status_t printCacheStats(Cache* cache, unsigned int style) {
    cache_status_t cache_status = {
        .error_code = CACHE_SUCCESS
    };
    
    // Check for null pointer
    if (cache == NULL) {
        cache_status.error_code = CACHE_IS_NULL;
        return cache_status;
    }

    if (style == 1) {
        // Output for Part 1
        printf("Total Requests: %zu\n", cache->requests);
        printf("     Miss Rate: %.2f%%\n", ((float)cache->misses / (float)cache->requests) * 100);
        printf("------------------------------------------------------------\n");

    } else if (style == 2) {
        // Output for Part 2
        printf("\nCache Layer: %u\n", cache->layer);
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

    return cache_status;
}

// ========================================================================
//                           Request Functions
// ========================================================================

/**
 * @brief Allocates memory for a memory request object
 * 
 * @param request Double pointer to request instance to allocate
 * @return request_status_t Status structure with error code
 */
request_status_t allocateRequest(Request** request) {
    request_status_t request_status = {
        .error_code = REQUEST_SUCCESS
    };

    // Allocate memory for Request
    *request = (Request*)malloc(sizeof(Request));
    if (*request == NULL) {
        request_status.error_code = REQUEST_ALLOCATION_FAILED;
    }

    return request_status;
}

/**
 * @brief Deallocates memory for a request given its pointer
 * 
 * @param request Pointer to request to be free'd
 */
void destroyRequest(Request* request) {
    // Check for null pointer
    if (request != NULL) {
        free(request);
    }

    // Debug Print
    if (DEBUG) {
        printf("Request successfully deleted\n");
    }
}

/**
 * @brief Parses trace input and formats memory request for cache processing
 * 
 * @param request Pointer to request object to populate
 * @param cache Pointer to target cache for address field calculations
 * @param buffer Input string containing trace data (format: @<I/D><R/W><hex-address>)
 * @return request_status_t Status structure with error code
 */
request_status_t formatRequest(Request* request, Cache* cache, const char* buffer) {
    request_status_t request_status = {
        .error_code = REQUEST_SUCCESS
    };

    // Check for null request
    if (request == NULL) {
        request_status.error_code = REQUEST_IS_NULL;
        return request_status;
    }

    // Check for null cache
    if (cache == NULL) {
        request_status.error_code = REQUEST_ON_NULL_CACHE;
        return request_status;
    }

    // Assign address field sizes based on cache
    request->address.tag_size = cache->tag_size;
    request->address.index_size = cache->index_size;
    request->address.offset_size = cache->offset_size;

    // Buffer format: <I/D><R/W><hex-address>
    // Assign reference type based on trace
    if (buffer[0] == 'I') request->ref_type = INSTRUCTION;
    else if (buffer[0] == 'D') request->ref_type = DATA;
    else {
        request_status.error_code = REQUEST_INVALID_REFERENCE_TYPE;
        request_status.ref_type = buffer[0];
        return request_status;
    }

    // Assign access type based on trace
    if (buffer[1] == 'R') request->access_type = 'R';
    else if (buffer[1] == 'W') request->access_type = 'W';
    else {
        request_status.error_code = REQUEST_INVALID_ACCESS_TYPE;
        request_status.access_type = buffer[1];
        return request_status;
    }

    // Format hex address and ensure it's proper
    if (sscanf(buffer + 2, "%x", &request->address.hex) != 1) {
        request_status.error_code = REQUEST_INVALID_REFERENCE_TYPE;
        return request_status;
    }

    // Convert address to binary representation
    itob(request->address.binary, request->address.hex);
    
    // Ensure tag buffer has enough space
    if (request->address.tag_size >= INSTRUCTION_SIZE) {
        request_status.error_code = REQUEST_INVALID_REFERENCE_TYPE;
        return request_status;
    }
    
    // Retrieve and copy tag, index, and offset
    strncpy(request->address.tag, 
            request->address.binary, // start at index 0
            request->address.tag_size
    );
    request->address.tag[request->address.tag_size] = '\0'; // end with null term

    strncpy(request->address.index, 
            request->address.binary + request->address.tag_size, // skip tag
            request->address.index_size
    );
    request->address.index[request->address.index_size] = '\0'; // end with null term

    strncpy(request->address.offset, 
            request->address.binary + request->address.tag_size + request->address.index_size, // skip tag + index
            request->address.offset_size
    );
    request->address.offset[request->address.offset_size] = '\0'; // end with null term

    // Debug Print
    if (DEBUG) {
        printf( "\nRequest Formatted:\n"
                  "-------------------\n"
                  "Reference Type: %c\n"
                  "Access Type: %c\n"
                  "Cache Layer: %u\n"
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
                  request->address.tag, btoi(request->address.tag),
                  request->address.index, btoi(request->address.index),
                  request->address.offset, btoi(request->address.offset));
    }

    return request_status;
}

/**
 * @brief Processes a memory request through the cache hierarchy
 * 
 * @param request Pointer to request object containing memory operation details
 * @param cache Pointer to target cache layer
 * @param data_found Output flag indicating cache hit (true) or miss (false)
 * @return request_status_t Status structure with error code
 */
request_status_t processRequest(Request* request, Cache* cache, bool* data_found) {
    request_status_t request_status = {
        .error_code = REQUEST_SUCCESS
    };

    // Check for null pointer
    if (cache == NULL) {
        request_status.error_code = REQUEST_ON_NULL_CACHE;
        return request_status;
    } else if (request == NULL) {
        request_status.error_code = REQUEST_IS_NULL;
        return request_status;
    }

    // Process request based on access type
    if (request->access_type == 'R') {
        request_status = readData(cache, request, data_found);
    } else if (request->access_type == 'W') {
        request_status = writeData(cache, request, data_found);
    } else {
        request_status.error_code = REQUEST_INVALID_ACCESS_TYPE;
        request_status.access_type = request->access_type;
    }

    return request_status;
}

/**
 * @brief Reads data from a cache at the address specified in the request.
 *        (This is where hit/miss stats are counted)
 *        
 * 
 * @param cache Pointer to cache layer the request will be performed on
 * @param request Pointer to request to be processed
 * @param data_found Output flag indicating cache hit (true) or miss (false)
 */
request_status_t readData(Cache* cache, Request* request, bool* data_found) {
    request_status_t request_status = {
        .error_code = REQUEST_SUCCESS
    };

    // Convert index to integer
    size_t index = btoi(request->address.index);

    // Check if index is within bounds
    if (index >= cache->num_lines) {
        request_status.error_code = REQUEST_INDEX_OUT_OF_BOUNDS;
        return request_status;
    }

    // Create reference to line for clarity
    Line* line = &cache->lines[index];

    // Check if the tag matches
    if (strcmp(line->tag, request->address.tag) == 0) {
        cache->hits++;
        *data_found = true;

        // Debug print
        if (DEBUG) {
            fprintf(stdout, "Hit! address: %x\n", request->address.hex);
        }
    } else {
        cache->misses++; 
        if (line->dirty == true) {
            cache->read_to_write++;
        }

        // Load the new tag, mark as clean
        strcpy(line->tag, request->address.tag);
        line->dirty = false;

        // Debug print
        if (DEBUG) {
            fprintf(stdout, "Miss! address: %x\n", request->address.hex);
        }
    }

    return request_status;
}

/**
 * @brief Writes data from a cache at the address specified in the request
 *        (This is where hit/miss stats are counted)
 * 
 * @param cache Pointer to cache layer the request will be performed on 
 * @param request Pointer to request to be processed 
 * @param data_found Output flag indicating cache hit (true) or miss (false)
 */
request_status_t writeData(Cache* cache, Request* request, bool* data_found) {
    request_status_t request_status = {
        .error_code = REQUEST_SUCCESS
    };

    // Convert index to integer
    size_t index = btoi(request->address.index);

    // Check if index is within bounds
    if (index >= cache->num_lines) {
        request_status.error_code = REQUEST_INDEX_OUT_OF_BOUNDS;
        return request_status;
    }

    // Create reference to line for clarity
    Line* line = &cache->lines[index];

    // If line found in cache
    if (strcmp(line->tag, request->address.tag) == 0) {
        cache->hits++;
        *data_found = true;
        line->dirty = true;  // data is now modified

        // Debug print
        if (DEBUG) {
            fprintf(stdout, "Hit! address: %x\n", request->address.hex);
        }

    } else {
        cache->misses++;
        if (line->dirty == true) {
            cache->write_to_write++;
        }

        // Load the new tag, mark as clean
        strcpy(line->tag, request->address.tag);
        line->dirty = false;

        // Debug print
        if (DEBUG) {
            fprintf(stdout, "Miss! address: %x\n", request->address.hex);
        }
    }

    return request_status;
}


//========================================================================
//                         Error Handling      
//========================================================================

/**
 * @brief Handles parameter validation errors and displays diagnostic messages
 * 
 * @param error Parameter error status structure containing error details
 * @return int Always returns EXIT_FAILURE to indicate program termination
 */
int parameter_error_handler(parameter_status_t error) {
    fprintf(stderr, "Error: ");

    switch(error.error_code) {

        case PARAMETER_SUCCESS:
            fprintf(stderr, "PARAMETER_SUCCESS unintentionally passed to handler\n");
            break;

        case PARAMETER_INVALID_ARG_COUNT:
            fprintf(stderr, "Invalid number of arguments. Expected 8, received %d.\n"
                            "Usage: %s <cache_type> <line_size> <cache_layers>" 
                            "<L1_size_B> <L2_size_B> <L3_size_B> <print_style>\n",
                             error.argument_count, error.executable_name);
            break;

        case PARAMETER_INVALID_CACHE_TYPE:
            fprintf(stderr, "Invalid cache type '%c'.\n", error.cache.type);
            break;

        case PARAMETER_INVALID_LINE_SIZE:
            fprintf(stderr, "Invalid line size '%zu'.\n", error.cache.line_size);
            break;

        case PARAMETER_INVALID_CACHE_LAYER_COUNT:
            fprintf(stderr, "Invalid cache layer count '%zu'.\n", error.cache.num_layers);
            break;

        case PARAMETER_INVALID_CACHE_SIZE:
            fprintf(stderr, "Invalid cache size '%zu' for layer '%u'.\n", 
                             error.cache.size, error.cache.layer);
            break;

        case PARAMETER_INVALID_PRINT_STYLE:
            fprintf(stderr, "Invalid print style '%u'.\n"
                            "Usage: 1 = standard print | 2 = debug print\n", 
                             error.print_style);
            break;
    }
    return EXIT_FAILURE;
}

/**
 * @brief Handles cache errors and displays diagnostic messages
 * 
 * @param error Cache error status structure containing error details
 * @return int Always returns EXIT_FAILURE to indicate program termination
 */
int cache_error_handler(cache_status_t error) {
    fprintf(stderr, "Error: ");

    switch(error.error_code) {

        case CACHE_SUCCESS:
            fprintf(stderr, "CACHE_SUCCESS unintentionally passed to handler\n");
            break;

        case CACHE_ALLOCATION_FAILED:
            fprintf(stderr, "Failed to allocate memory for cache with following attributes.\n"
                            "{Layer: %u | Cache Size: %zu | Line Size: %zu}\n", 
                            error.cache.layer, error.cache.size, error.cache.line_size);
            break;

        case CACHE_LINE_ALLOCATION_FAILED:
            fprintf(stderr, "Failed to allocate memory for cache lines of cache with following attributes.\n"
                            "{Layer: %u | Cache Size: %zu | Line Size: %zu}\n",
                            error.cache.layer, error.cache.size, error.cache.size);
            break;

        case CACHE_IS_NULL:   
            fprintf(stderr, "Cache instance is null.\n");
            break;

        case CACHE_SIZE_NOT_POWER_OF_TWO:
        fprintf(stderr, "Cache expects number of lines to be power of two, recieved \"%zu\"\nPlease adjust line_size\n",
                        error.cache.num_lines);
        break;
    }
    
    return EXIT_FAILURE;
}

/**
 * @brief Handles request errors and displays diagnostic messages
 * 
 * @param error Request error status structure containing error details
 * @return int Always returns EXIT_FAILURE to indicate program termination
 */
int request_error_handler(request_status_t error) {
    fprintf(stderr, "Error: ");

    switch(error.error_code) {

        case REQUEST_SUCCESS:
            fprintf(stderr, "REQUEST_SUCCESS unintentionally passed to handler\n");
            break;

        case REQUEST_ALLOCATION_FAILED:
            fprintf(stderr, "Failed to allocate memory for request.\n");
            break;

        case REQUEST_INVALID_REFERENCE_TYPE:
            fprintf(stderr, "Invalid reference type '%c'.\n", error.ref_type);
            break;

        case REQUEST_INVALID_ACCESS_TYPE:
            fprintf(stderr, "Invalid access type '%c'.\n", error.access_type);
            break;

        case REQUEST_IS_NULL:
            fprintf(stderr, "Request instance is null.\n");
            break;

        case REQUEST_ON_NULL_CACHE:   
            fprintf(stderr, "Cannot process request on a NULL cache.\n");
            break;

        case REQUEST_INDEX_OUT_OF_BOUNDS:   
            fprintf(stderr, "Request of index \"%s\" out of bounds\n", error.index);
            break;
    }
    return EXIT_FAILURE;
}


// ========================================================================
//                         Additional Helpers
// ========================================================================

/**
 * @brief Converts 32-bit integer to binary string representation
 * 
 * @param binary Pointer to binary string
 * @param hex Hex value of number to be converted
 */
void itob(char* binary, unsigned int hex) {
    for (int i = 31; i >= 0; i--) {
        binary[31 - i] = ((hex >> i) & 1) ? '1' : '0';
    }
    binary[32] = '\0';
}

/**
 * @brief Converts binary string to integer value
 * 
 * @param binary Null-terminated binary string to convert
 * @return int Decimal representation of binary input
 */
unsigned int btoi(const char* binary) {
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
 * @brief Checks if value is a power of two
 * 
 * @param n Value to check
 * @return true 
 * @return false 
 */
bool isPowerOfTwo(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

/*
===========================================================================
 PROJECT: Direct-Mapped Write-Back Cache [Trace Driven Simulation]
===========================================================================
 NAME: Tyler Neal
 DATE: 01/09/2025
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
#include "cache.h"
#include "error.h"

// Global Variables
int cache_layers = 0;
 
// ========================================================================
//                               Main
// ========================================================================

/**
 * @brief Simulates a (1-3) layer direct-mapped write-back cache and prints statistics given inputed Trace files
 * 
 * @param argc - number of arguments
 * @param argv - array of arguments in char pointer format
 * @return int - 0 on success : >= 1 on failure
 */
int main(int argc, char* argv[]) {

    // ========== Set up Timing Elements ==========
    clock_t start_time, end_time;
    double elapsed_time;
    start_time = clock();


    // ========== Verify Command-line Arguments ==========
    parameter_error_info_t parameter_error = {
        .error_code = PARAMETER_SUCCESS,
        .executable_name = argv[0]
    };

    // Check argument count
    if (argc < 8) {
        parameter_error.error_code = PARAMETER_INVALID_ARG_COUNT;
        parameter_error.argument_count = argc;
        return parameter_error_handler(parameter_error);
    }

    // Parse command-line arguments
    Cache_Type cache_type = (Cache_Type)*argv[1];
    if (cache_type != UNIFIED && cache_type != INSTRUCTION && cache_type != DATA) {
        parameter_error.error_code = PARAMETER_INVALID_CACHE_TYPE;
        parameter_error.cache_type = cache_type;
        return parameter_error_handler(parameter_error);
    }
    
    int line_size = (atoi(argv[2])*4);  // line size parameter is in unit of words. We mutliply by 4 as 1 word = 4 bytes
    if (line_size < 4 || line_size > 64) {
        parameter_error.error_code = PARAMETER_INVALID_LINE_SIZE;
        parameter_error.line_size = line_size;
        return parameter_error_handler(parameter_error);
    }

    cache_layers = atoi(argv[3]);
    if (cache_layers < 1 || cache_layers > 3) {
        parameter_error.error_code = PARAMETER_INVALID_CACHE_LAYER_COUNT;
        parameter_error.cache_layers = cache_layers;
        return parameter_error_handler(parameter_error);
    }
    
    int L1_size = (atoi(argv[4])*1024); // cache sizes are in unit of KB. We multiply by 1024 to convert to bytes
    if (L1_size < 0 || L1_size % line_size != 0) {
        parameter_error.error_code = PARAMETER_INVALID_CACHE_SIZE;
        parameter_error.cache_size = L1_size;
        parameter_error.layer = 1;
        return parameter_error_handler(parameter_error);
    }
    
    int L2_size = (atoi(argv[5])*1024);
    if (L2_size < 0 || L2_size % line_size != 0) {
        parameter_error.error_code = PARAMETER_INVALID_CACHE_SIZE;
        parameter_error.cache_size = L2_size;
        parameter_error.layer = 2;
        return parameter_error_handler(parameter_error);
    }
    
    int L3_size = (atoi(argv[6])*1024); 
    if (L3_size < 0 || L3_size % line_size != 0) {
        parameter_error.error_code = PARAMETER_INVALID_CACHE_SIZE;
        parameter_error.cache_size = L3_size;
        parameter_error.layer = 3;
        return parameter_error_handler(parameter_error);
    }

    int print_style = atoi(argv[7]);    // style of 1 prints total request and miss rate, while style 2 prints more detailed view
    if (print_style != 1 && print_style != 2) {
        parameter_error.error_code = PARAMETER_INVALID_PRINT_STYLE;
        parameter_error.print_style = print_style;
        return parameter_error_handler(parameter_error);
    }


    // Declare cache layers
    Cache* cache[3] = {NULL, NULL, NULL}; // each index represents a cache layer where layer number = index + 1
    int layer_sizes[3] = {L1_size, L2_size, L3_size};
    cache_error_info_t cache_setup_error;

    // Allocate and populate cache layers
    for (int i = 0; i < cache_layers; i++) {
        cache_setup_error = setupCache(&cache[i], i+1, layer_sizes[i], line_size);
        if (cache_setup_error.error_code != CACHE_SUCCESS) {
            cache_error_handler(cache_setup_error);
            return cache_setup_error.error_code;
        }
    }


    // ========== Process Requests Until End of File ==========
    char current_char;
    char buffer[12]; // traces are 11 char's long + null terminator
    buffer[11] = '\0';
    bool data_found = false;

    // Initialize request
    Request* request;
    request_error_info_t request_error = allocateRequest(&request);
    if (request_error.error_code != REQUEST_SUCCESS) {
        request_error_handler(request_error);
        return request_error.error_code;
    }

    // Process each trace in the input file
    while ((current_char = getchar()) != EOF) {
        if (current_char == '@') {
            fgets(buffer, sizeof(buffer), stdin); // reads trace format: @<I/D><R/W><hex-address>

            // Try each cache layer until data is found
            for (int i = 0; i < cache_layers; i++) {
                request_error = formatRequest(request, cache[i], buffer);
                if (request_error.error_code != REQUEST_SUCCESS) {
                    request_error_handler(request_error);
                    return request_error.error_code;
                }

                // Process request if cache type matches or if cache is unified
                if (request->ref_type == cache_type || cache_type == UNIFIED) {
                    cache[i]->requests++;
                    request_error = processRequest(request, cache[i], &data_found);
                    if (request_error.error_code != REQUEST_SUCCESS) {
                        request_error_handler(request_error);
                        return request_error.error_code;
                    }
                    if (data_found) break; // exit loop on cache hit
                }
            }
            data_found = false;
        }
    }


    // ========== Calculate & Prints Cache Statistics ==========
    // Calculate miss rates
    float miss_rates[3];
    for (int i = 0; i < cache_layers; i++) {
        miss_rates[i] = ((float)cache[i]->misses / (float)cache[i]->requests)   ;
    }

    // Print cache statistics
    cache_error_info_t print_error;
    for (int i = 0; i < cache_layers; i++) {
        print_error = printCacheStats(cache[i], print_style);
        if (print_error.error_code != CACHE_SUCCESS) {
            cache_error_handler(print_error);
            return print_error.error_code;
        }
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
    for (int i = 0; i < cache_layers; i++) {
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
 * @brief Allocates memory for a cache and returns a pointer to it
 * 
 * @param cache_size - size of cache in bytes
 * @param line_size - size of each cache line in bytes
 * @param layer - layer the cache represents (e.g. L1, L2, L3)
 * @return Cache* - pointer to allocated cache
 */
cache_error_info_t allocateCache(Cache** cache, int cache_size, int line_size, int layer) {
    cache_error_info_t error_info = {
        .error_code = CACHE_SUCCESS,
        .layer = layer,
        .cache_size = cache_size,
        .line_size = line_size
    };

    // Check for null pointer
    if (cache == NULL) {
        error_info.error_code = CACHE_IS_NULL;
        return error_info;
    }

    // Allocate space for cache
    *cache = (Cache*)malloc(sizeof(Cache));
    if (*cache == NULL) {
        error_info.error_code = CACHE_ALLOCATION_FAILED;
        return error_info;
    }

    // Initialize cache struct values
    (*cache)->cache_size = cache_size;
    (*cache)->line_size = line_size;
    (*cache)->num_lines = (cache_size / line_size);
    (*cache)->layer = layer;
    (*cache)->requests = 0;
    (*cache)->hits = 0;
    (*cache)->misses = 0;
    (*cache)->read_to_write = 0;
    (*cache)->write_to_write = 0;

    // Memory allocation for cache lines
    (*cache)->lines = (Line*)malloc((*cache)->num_lines * sizeof(Line));
    if ((*cache)->lines == NULL) {
        error_info.error_code = CACHE_LINE_ALLOCATION_FAILED;
        return error_info;
    }
    for (int i = 0; i < (*cache)->num_lines; i++) {
        // Setting initial values for each cache line
        (*cache)->lines[i].dirty = 0;        // Marks line as initially clean (not modified)
        (*cache)->lines[i].tag[0] = 'x';     // Initial tag set to 'x' to represent uninitialized
    }

    if (DEBUG) {
        printf("Cache Created\n..............\nSize: %d bytes\nLine Count: %d\nLine Size: %d bytes\n", 
              (*cache)->cache_size, (*cache)->num_lines, (*cache)->line_size);
    }

    return error_info;
}

/**
 * @brief Deallocates memory for a cache given its pointer
 * 
 * @param cache - pointer to cache to be destroyed
 */
void destroyCache(Cache* cache) {
    // Check for null pointer
    if (cache != NULL) {
        if (cache->lines != NULL) {
            free(cache->lines);
        }
        free(cache);
    }

    if (DEBUG) {
        printf("Cache successfully deleted\n");
    }
}

/**
 * @brief Cacluates and stores the address field sizes for a given cache
 * 
 * @param layer - layer the cache represents (e.g. L1, L2, L3)
 * @param cache_size - size of cache in bytes
 * @param line_size - size of each cache line in bytes
 * @return Cache* - pointer to initialized cache
 */
cache_error_info_t setupCache(Cache** cache, int layer, int cache_size, int line_size) {
    cache_error_info_t error_info;
    error_info = allocateCache(cache, cache_size, line_size, layer);
    if (error_info.error_code != CACHE_SUCCESS) {
        return error_info;
    }

    // Calculate Address Field Sizes
    (*cache)->offset_size = (int)ceil(log2(line_size)); // we round up the value to ensure there are enough bits to address all lines
    (*cache)->index_size = (int)ceil(log2(cache_size / line_size));
    (*cache)->tag_size = INSTRUCTION_SIZE - (*cache)->index_size - (*cache)->offset_size;

    // Debug Printing
    if (DEBUG) {
        printf("\nCache Size: %d\nLine Size: %d\ncache->tag_size: %d\ncache->index_size: %d\ncache->offset_size: %d\n\n",
               cache_size, line_size, (*cache)->tag_size, (*cache)->index_size, (*cache)->offset_size);
    }

    return error_info;
}

/**
 * @brief Prints cache statistics, including cache configuration, as well as
 *        performance metrics like hit rate, miss rate, and request ratios.
 * 
 * @param cache - pointer to cache who's statistics will be printed
 * @param style - style of stat print (1 for simple view : 2 for more detailed stats)
 */
cache_error_info_t printCacheStats(Cache* cache, int style) {
    cache_error_info_t error_info = {
        .error_code = CACHE_SUCCESS
    };
    
    // Check for null pointer
    if (cache == NULL) {
        error_info.error_code = CACHE_IS_NULL;
        return error_info;
    }

    if (style == 1) {
        // Output for Part 1
        printf("Total Requests: %d\n", cache->requests);
        printf("     Miss Rate: %.2f%%\n", ((float)cache->misses / (float)cache->requests) * 100);
        printf("------------------------------------------------------------\n");

    } else if (style == 2) {
        // Output for Part 2
        printf("Cache Layer: L%d\n", cache->layer);
        printf("----------------\n");
        printf("Configuration:\n");
        printf("    Size: %d bytes\n", cache->cache_size);
        printf("    Line Size: %d bytes\n", cache->line_size);
        printf("    Line Count: %d\n", cache->num_lines);
        printf("Performance Metrics:\n");
        printf("    Total Requests: %d\n", cache->requests);
        printf("    Hits: %d\n", cache->hits);
        printf("    Misses: %d\n", cache->misses);
        printf("    Hit Rate: %.2f%%\n", ((float)cache->hits / (float)cache->requests) * 100);
        printf("    Miss Rate: %.2f%%\n", ((float)cache->misses / (float)cache->requests) * 100);
        printf("    Read to Write Ratio: %d\n", cache->read_to_write);
        printf("    Write to Write Ratio: %d\n", cache->write_to_write);
    }
    return error_info;
}

// ========================================================================
//                           Request Functions
// ========================================================================

/**
 * @brief Allocates memory for a memory request
 * 
 * @return Request* - pointer to allocated request
 */
request_error_info_t allocateRequest(Request** request) {
    request_error_info_t request_error = {
        .error_code = REQUEST_SUCCESS
    };

    // Check for null pointer
    if (request == NULL) {
        request_error.error_code = REQUEST_ON_NULL_CACHE;
        return request_error;
    }

    // Allocate memory for Request
    *request = (Request*)malloc(sizeof(Request));
    if (*request == NULL) {
        request_error.error_code = REQUEST_ALLOCATION_FAILED;
        return request_error;
    }

    return request_error;
}

/**
 * @brief Deallocates memory for a request given its pointer
 * 
 * @param request - pointer to request to be free'd
 */
void destroyRequest(Request* request) {
    // Check for null pointer
    if (request != NULL) {
        free(request);
    }
}

/**
 * @brief Formats the tag index and offset of a request given the cache it will query
 * 
 * @param request - pointer to request to be formated
 * @param cache - pointer to cache layer the request will be performed on
 * @param buffer - string buffer containing memory request (format: @<I/D><R/W><hex-address>)
 */
request_error_info_t formatRequest(Request* request, Cache* cache, const char* buffer) {
    request_error_info_t request_error = {
        .error_code = REQUEST_SUCCESS
    };

    // Assign reference type based on trace
    if (buffer[0] == 'I') request->ref_type = INSTRUCTION;
    else if (buffer[0] == 'D') request->ref_type = DATA;
    else {
        request_error.error_code = REQUEST_INVALID_REFERENCE_TYPE;
        request_error.ref_type = buffer[0];
        return request_error;
    }

    // Assign access type based on trace
    if (buffer[1] == 'R') request->access_type = 'R';
    else if (buffer[1] == 'W') request->access_type = 'W';
    else {
        request_error.error_code = REQUEST_INVALID_ACCESS_TYPE;
        request_error.access_type = buffer[1];
        return request_error;
    }

    // Fill in hex address
    sscanf(buffer + 2, "%x", &request->address);

    // Convert address to binary representation (placeholder function itob)
    char request_address_binary[33];
    char* binary_string = itob(request->address);
    if (binary_string == NULL) {
        request_error.error_code = REQUEST_BINARY_CONVERSION_FAILED;
        request_error.address = request->address;
        return request_error;
    }
    strcpy(request_address_binary, binary_string);
    free(binary_string);

    // Retrieve and copy tag, index, and offset
    strncpy(request->tag, request_address_binary, cache->tag_size);
    request->tag[cache->tag_size] = '\0';
    strncpy(request->index, request_address_binary + cache->tag_size, cache->index_size);
    request->index[cache->index_size] = '\0';
    strncpy(request->offset, request_address_binary + cache->tag_size + cache->index_size, cache->offset_size);
    request->offset[cache->offset_size] = '\0';

    return request_error;
}

/**
 * @brief Takes a request and sends the read / write request to the supplied cache.
 *        Based on request type passes request to readData() or writeData()
 * 
 * @param request - pointer to request to be processed
 * @param cache - pointer to cache layer the request will be performed on
 * @param data_found - flag that represents a hit made (true if data is found, false otherwise)
 */
request_error_info_t processRequest(Request* request, Cache* cache, bool* data_found) {
    request_error_info_t request_error = {
        .error_code = REQUEST_SUCCESS
    };

    // Check for null pointer
    if (cache == NULL) {
        request_error.error_code = REQUEST_ON_NULL_CACHE;
        return request_error;
    } else if (request == NULL) {
        request_error.error_code = REQUEST_IS_NULL;
        return request_error;
    }

    // Process request based on access type
    request_error_info_t mem_access_info = {
        .error_code = REQUEST_SUCCESS
    };
    if (request->access_type == 'R') {
        mem_access_info = readData(cache, request, data_found);
        if (mem_access_info.error_code != REQUEST_SUCCESS) {
            request_error.error_code = mem_access_info.error_code;
            return request_error;
        }
    } else if (request->access_type == 'W') {
        mem_access_info = writeData(cache, request, data_found);
        if (mem_access_info.error_code != REQUEST_SUCCESS) {
            request_error.error_code = mem_access_info.error_code;
            return request_error;
        }
    } else {
        request_error.error_code = REQUEST_INVALID_ACCESS_TYPE;
        request_error.access_type = request->access_type;
        return request_error;
    }

    return request_error;
}

/**
 * @brief Reads data from a cache at the address specified in the request.
 *        (This is where hit / miss stats are counted)
 *        
 * 
 * @param cache - pointer to cache layer the request will be performed on
 * @param request - pointer to request to be processed
 * @param data_found - flag that represents a hit made (true if data is found : false otherwise)
 */
request_error_info_t readData(Cache* cache, Request* request, bool* data_found) {
    request_error_info_t request_error = {
        .error_code = REQUEST_SUCCESS
    };

    // Convert index to integer
    int index = btoi(request->index);

    // Check if index is within bounds
    if (index >= 0 && index <= cache->num_lines) {
        Line* line = &cache->lines[index];

        // Check if the tag matches
        if (strcmp(line->tag, request->tag) == 0) {
            cache->hits++;
            *data_found = true;
        } else {
            cache->misses++; 
            if (line->dirty == 1) {
                cache->read_to_write++;
            }

            // Load the new tag, mark as clean
            strcpy(line->tag, request->tag);
            line->dirty = 0;
        }
    }
    return request_error;
}

/**
 * @brief Writes data from a cache at the address specified in the request
 *        (This is where hit / miss stats are counted)
 * 
 * @param cache - pointer to cache layer the request will be performed on 
 * @param request - pointer to request to be processed 
 * @param data_found - flag that represents a hit made (TRUE if data is found : FALSE if data wasnt found)
 */
request_error_info_t writeData(Cache* cache, Request* request, bool* data_found) {
    request_error_info_t request_error = {
        .error_code = REQUEST_SUCCESS
    };

    // Convert index to integer
    int index = btoi(request->index);

    // Check if index is within bounds
    if (index >= 0 && index <= cache->num_lines) {
        Line* line = &cache->lines[index];

        // If line found in cache
        if (strcmp(line->tag, request->tag) == 0) {
            cache->hits++;
            line->dirty = 1;  // Data is now modified
            *data_found = true;
        } else {
            cache->misses++;
            if (line->dirty == 1) {
                cache->write_to_write++;
            }

            // Load the new tag, mark as clean
            strcpy(line->tag, request->tag);
            line->dirty = 1;
        }
    }
    return request_error;
}


//========================================================================
//                         Additional Helpers       
//========================================================================

/**
 * @brief Handles parameter errors and prints appropriate error message
 * 
 * @param error - parameter error information
 */
int parameter_error_handler(parameter_error_info_t error) {
    fprintf(stderr, "Error: ");
    switch(error.error_code) {
        case PARAMETER_SUCCESS:
            fprintf(stderr, "PARAMETER_SUCCESS unintentionally passed to handler\n");
            break;
        case PARAMETER_INVALID_ARG_COUNT:
            fprintf(stderr, "Invalid number of arguments. Expected 8, received %d.\n"
                            "Usage: %s <cache_type> <line_size> <cache_layers>" 
                            "<L1_size> <L2_size> <L3_size> <print_style>\n",
                            error.argument_count, error.executable_name);
            break;
        case PARAMETER_INVALID_CACHE_TYPE:
            fprintf(stderr, "Invalid cache type '%c'.\n", error.cache_type);
            break;
        case PARAMETER_INVALID_LINE_SIZE:
            fprintf(stderr, "Invalid line size '%d'.\n", error.line_size);
            break;
        case PARAMETER_INVALID_CACHE_LAYER_COUNT:
            fprintf(stderr, "Invalid cache layer count '%d'.\n", error.cache_layers);
            break;
        case PARAMETER_INVALID_CACHE_SIZE:
            fprintf(stderr, "Invalid cache size '%d' for layer '%d'.\n", 
                            error.cache_size, error.layer);
            break;
        case PARAMETER_INVALID_PRINT_STYLE:
            fprintf(stderr, "Invalid print style '%d'.\n"
                            "Usage: 1 = standard print | 2 = debug print\n", 
                            error.print_style);
            break;
    }
    return EXIT_FAILURE;
}

/**
 * @brief Handles cache errors and prints appropriate error message
 * 
 * @param error - cache error information
 */
int cache_error_handler(cache_error_info_t error) {
    fprintf(stderr, "Error: ");
    switch(error.error_code) {
        case CACHE_SUCCESS:
            fprintf(stderr, "CACHE_SUCCESS unintentionally passed to handler\n");
            break;
        case CACHE_ALLOCATION_FAILED:
            fprintf(stderr, "Failed to allocate memory for cache with following attributes.\n"
                            "{Layer: %d | Cache Size: %d | Line Size: %d}\n", 
                            error.layer, error.cache_size, error.line_size);
            break;
        case CACHE_LINE_ALLOCATION_FAILED:
            fprintf(stderr, "Failed to allocate memory for cache lines of cache with following attributes.\n"
                            "{Layer: %d | Cache Size: %d | Line Size: %d}\n",
                            error.layer, error.cache_size, error.line_size);
            break;
        case CACHE_IS_NULL:   
            fprintf(stderr, "Cache instance is null.\n");
            break;
    }
    
    return EXIT_FAILURE;
}

/**
 * @brief Handles request errors and prints appropriate error message
 * 
 * @param error - request error information
 */
int request_error_handler(request_error_info_t error) {
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
        case REQUEST_BINARY_CONVERSION_FAILED:
            fprintf(stderr, "Failed to convert address '%d' to binary.\n", error.address);
            break;
        case REQUEST_IS_NULL:
            fprintf(stderr, "Request instance is null.\n");
            break;
        case REQUEST_ON_NULL_CACHE:   
            fprintf(stderr, "Cannot process request on a NULL cache.\n");
            break;
    }
    return EXIT_FAILURE;
}


// ========================================================================
//                         Additional Helpers
// ========================================================================

/**
 * @brief Converts an integer to binary
 * 
 * @param num - number to be converted
 * @return char* - binary of number in string format
 */
char* itob(int num) {
    size_t numBits = sizeof(int) * 8;
    char* binaryStr = (char*)malloc(numBits + 1);

    // Check for null pointer
    if (binaryStr == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    // Add null terminator
    binaryStr[numBits] = '\0';

    // Convert integer to binary string
    for (size_t i = 0; i < numBits; ++i) {
        binaryStr[numBits - 1 - i] = (num & (1 << i)) ? '1' : '0';
    }

    return binaryStr;
}

/**
 * @brief Converts binary to an integer
 * 
 * @param binary - binary value of a number
 * @return int - integer representation of binary
 */
int btoi(const char* binary) {
    int value = 0;
    size_t len = strlen(binary);

    // Convert binary string to integer
    for (size_t i = 0; i < len; ++i) {
        value <<= 1;    // shift the current value to the left by one bit
        if (binary[i] == '1') {
            value += 1; // add 1 if the current binary digit is 1
        } else if (binary[i] != '0') {
            fprintf(stderr, "Invalid character '%c' in binary string.\n", binary[i]);
            return 0;   // return 0 or an appropriate error value
        }
    }

    return value;
}


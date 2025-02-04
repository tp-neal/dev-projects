/*
===========================================================================
 PROJECT: Direct-Mapped Write-Back Cache [Trace Driven Simulation]
===========================================================================
 NAME : Tyler Neal
 USER ID : tpneal
 DATE: 01/09/2025
 FILE NAME : cache.h
 DESCRIPTION:
    This header file declares the structures necessary to model a direct-mapped
    write-back cache. It provides declarations for cache blocks, sets, and the
    entire cache structure.

 CONTENTS:
    - Cache Definitions for Memory Request Times and Address Calculation
    - Cache Structures and Enums
    - Request Structures and Enums
    - Cache Function Declarations

===========================================================================
*/

#ifndef CACHE_H
#define CACHE_H

#include <stdbool.h>
#include "error.h"

/* ===================================================================
                            Definitions
=================================================================== */

// Memory request times (ms) - used in statistic calculations
#define HIT_TIME_L1 1
#define HIT_TIME_L2 16
#define HIT_TIME_L3 64
#define MEM_ACCESS_TIME 100

// According to our implementation, these values wont exceed their defined max
#define INSTRUCTION_SIZE 32
#define MAX_TAG_SIZE 21
#define MAX_INDEX_SIZE 16
#define MAX_OFFSET_SIZE 5

// Used for debug printing
#define DEBUG false

/* ==================================================================/
/                       Structures and Enums                         /
/================================================================== */

/***************| Cache |***************/

typedef enum {
  UNIFIED = 'U', // Both Data and Instruction
  DATA = 'D', // Data
  INSTRUCTION = 'I'  // Instruction
} Cache_Type;

typedef struct Line {
  int dirty; // Represents the dirty bit
  char tag[MAX_TAG_SIZE];
} Line;

typedef struct Cache {
  // Cache Details
  int cache_size; // (bytes)
  int line_size;  // (bytes)
  int num_lines;  // (lines/block)
  int layer;      // (e.g L1, L2, L3)
  Line *lines;

  // Feild Sizes
  int tag_size;
  int index_size;
  int offset_size;

  // Recorded Metrics
  int requests;
  int hits;
  int misses;
  int read_to_write;
  int write_to_write;
} Cache;

/***************| Request |***************/

typedef struct Request {
  Cache_Type ref_type;
  char access_type;

  unsigned int address; // Hex address formatted as int
  char tag[MAX_TAG_SIZE+1]; // +1 for null terminator
  char index[MAX_INDEX_SIZE+1];
  char offset[MAX_OFFSET_SIZE+1];
} Request;

/* ===================================================================
                    Cache Function Declarations
=================================================================== */

/***************| Cache |***************/
/**
 * @brief Allocates memory for a cache and returns a pointer to it
 * 
 * @param cache_size - size of cache in bytes
 * @param line_size - size of each cache line in bytes
 * @param layer - layer the cache represents (e.g. L1, L2, L3)
 * @return Cache* - pointer to allocated cache
 */
cache_error_info_t allocateCache(Cache** cache, int cache_size, int line_size, int layer);

/**
 * @brief Deallocates memory for a cache given its pointer
 * 
 * @param cache - pointer to cache to be destroyed
 */
void destroyCache(Cache* cache);

/**
 * @brief Cacluates and stores the address field sizes for a given cache
 * 
 * @param layer - layer the cache represents (e.g. L1, L2, L3)
 * @param cache_size - size of cache in bytes
 * @param line_size - size of each cache line in bytes
 * @return Cache* - pointer to initialized cache
 */
cache_error_info_t setupCache(Cache** cache, int layer, int cache_size, int line_size);

/**
 * @brief Prints cache statistics, including cache configuration, as well as
 *        performance metrics like hit rate, miss rate, and request ratios.
 * 
 * @param cache - pointer to cache who's statistics will be printed
 * @param style - style of stat print (1 for simple view : 2 for more detailed stats)
 */
cache_error_info_t printCacheStats(Cache* cache, int style);


/***************| Request |***************/
/**
 * @brief Allocates memory for a memory request
 * 
 * @return Request* - pointer to allocated request
 */
request_error_info_t allocateRequest(Request** request);

/**
 * @brief Deallocates memory for a request given its pointer
 * 
 * @param request - pointer to request to be free'd
 */
void destroyRequest(Request* request);

/**
 * @brief Formats the tag index and offset of a request given the cache it will query
 * 
 * @param request - pointer to request to be formated
 * @param cache - pointer to cache layer the request will be performed on
 * @param buffer - string buffer containing memory request (format: @<I/D><R/W><hex-address>)
 */
request_error_info_t formatRequest(Request* request, Cache* cache, const char* buffer);

/**
 * @brief Takes a request and sends the read / write request to the supplied cache.
 *        Based on request type passes request to readData() or writeData()
 * 
 * @param request - pointer to request to be processed
 * @param cache - pointer to cache layer the request will be performed on
 * @param data_found - flag that represents a hit made (true if data is found, false otherwise)
 */
request_error_info_t processRequest(Request* request, Cache* cache, bool* data_found);

/**
 * @brief Reads data from a cache at the address specified in the request.
 *        (This is where hit / miss stats are counted)
 *        
 * 
 * @param cache - pointer to cache layer the request will be performed on
 * @param request - pointer to request to be processed
 * @param data_found - flag that represents a hit made (true if data is found : false otherwise)
 */
request_error_info_t readData(Cache* cache, Request* request, bool* data_found);

/**
 * @brief Writes data from a cache at the address specified in the request
 *        (This is where hit / miss stats are counted)
 * 
 * @param cache - pointer to cache layer the request will be performed on 
 * @param request - pointer to request to be processed 
 * @param data_found - flag that represents a hit made (TRUE if data is found : FALSE if data wasnt found)
 */
request_error_info_t writeData(Cache* cache, Request* request, bool* data_found);


/***************| Helper |***************/
/**
 * @brief Converts an integer to binary
 * 
 * @param num - number to be converted
 * @return char* - binary of number in string format
 */
char* itob(int num);

/**
 * @brief Converts binary to an integer
 * 
 * @param binary - binary value of a number
 * @return int - integer representation of binary
 */
int btoi(const char* binary);


#endif // CACHE_H

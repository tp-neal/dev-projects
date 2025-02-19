
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
typedef struct Line {
  bool dirty; // represents the dirty bit
  char tag[INSTRUCTION_SIZE];
} Line;

typedef enum {
	UNIFIED = 'U',     // Both Data and Instruction
	DATA = 'D',        // Data
	INSTRUCTION = 'I'  // Instruction
} Cache_Type;

typedef struct Cache {
  // Cache Details
  unsigned int layer;      // (e.g L1, L2, L3)
  size_t cache_size; // (bytes)
  size_t line_size;  // (bytes)
  size_t num_lines;  // (lines/block)
  Line *lines;

  // Feild Sizes
  unsigned int tag_size;
  unsigned int index_size;
  unsigned int offset_size;

  // Recorded Metrics
  size_t requests;
  size_t hits;
  size_t misses;
  size_t read_to_write;
  size_t write_to_write;
} Cache;

/***************| Request |***************/
typedef struct Address { 
  unsigned int hex;
  char binary[33]; // temp size

  char tag[INSTRUCTION_SIZE+1]; // +1 for null terminator
  unsigned int tag_size;

  char index[INSTRUCTION_SIZE+1];
  unsigned int index_size;

  char offset[INSTRUCTION_SIZE+1];
  unsigned int offset_size;

} Address;

typedef struct Request {
  Cache_Type ref_type;
  char access_type;
  Address address;
} Request;

/* ===================================================================
                      Function Declarations
=================================================================== */

/***************| Cache |***************/
/**
 * @brief Allocates memory for a cache structure
 * 
 * @param cache Double pointer to cache instance to allocate
 * @param layer Cache layer number (1-3)
 * @param cache_size Total size of cache in bytes
 * @param line_size Size of each cache line in bytes
 * @return cache_status_t Status structure with error code and cache metadata
 */
cache_status_t allocateCache(Cache** cache, unsigned int layer, size_t cache_size, size_t line_size);

/**
 * @brief Deallocates memory for a cache given its pointer
 * 
 * @param cache Pointer to cache to be destroyed
 */
void destroyCache(Cache* cache);

/**
 * @brief Initializes cache structure and calculates address field sizes
 * 
 * @param cache Double pointer to cache instance to initialize
 * @param layer Cache layer number (1-3)
 * @param cache_size Total size of cache in bytes
 * @param line_size Size of each cache line in bytes
 * @return cache_status_t Status structure with error code and cache metadata
 */
cache_status_t setupCache(Cache** cache, unsigned int layer, size_t cache_size, size_t line_size);

/**
 * @brief Prints cache statistics and configuration details
 * 
 * @param cache Pointer to cache instance to display stats for
 * @param style Output style (1 = compact, 2 = verbose)
 * @return cache_status_t Status structure with error code
 */
cache_status_t printCacheStats(Cache* cache, unsigned int style);


/***************| Request |***************/
/**
 * @brief Allocates memory for a memory request object
 * 
 * @param request Double pointer to request instance to allocate
 * @return request_status_t Status structure with error code
 */
request_status_t allocateRequest(Request** request);

/**
 * @brief Deallocates memory for a request given its pointer
 * 
 * @param request Pointer to request to be free'd
 */
void destroyRequest(Request* request);

/**
 * @brief Parses trace input and formats memory request for cache processing
 * 
 * @param request Pointer to request object to populate
 * @param cache Pointer to target cache for address field calculations
 * @param buffer Input string containing trace data (format: @<I/D><R/W><hex-address>)
 * @return request_status_t Status structure with error code
 */
request_status_t formatRequest(Request* request, Cache* cache, const char* buffer);

/**
 * @brief Processes a memory request through the cache hierarchy
 * 
 * @param request Pointer to request object containing memory operation details
 * @param cache Pointer to target cache layer
 * @param data_found Output flag indicating cache hit (true) or miss (false)
 * @return request_status_t Status structure with error code
 */
request_status_t processRequest(Request* request, Cache* cache, bool* data_found);

/**
 * @brief Reads data from a cache at the address specified in the request.
 *        (This is where hit/miss stats are counted)
 *        
 * 
 * @param cache Pointer to cache layer the request will be performed on
 * @param request Pointer to request to be processed
 * @param data_found Output flag indicating cache hit (true) or miss (false)
 */
request_status_t readData(Cache* cache, Request* request, bool* data_found);

/**
 * @brief Writes data from a cache at the address specified in the request
 *        (This is where hit/miss stats are counted)
 * 
 * @param cache Pointer to cache layer the request will be performed on 
 * @param request Pointer to request to be processed 
 * @param data_found Output flag indicating cache hit (true) or miss (false)
 */
request_status_t writeData(Cache* cache, Request* request, bool* data_found);


/***************| Error Handling |***************/
/**
 * @brief Handles parameter validation errors and displays diagnostic messages
 * 
 * @param error Parameter error status structure containing error details
 * @return int Always returns EXIT_FAILURE to indicate program termination
 */
int parameter_error_handler(parameter_status_t error);

/**
 * @brief Handles cache errors and displays diagnostic messages
 * 
 * @param error Cache error status structure containing error details
 * @return int Always returns EXIT_FAILURE to indicate program termination
 */
int cache_error_handler(cache_status_t error);

/**
 * @brief Handles request errors and displays diagnostic messages
 * 
 * @param error Request error status structure containing error details
 * @return int Always returns EXIT_FAILURE to indicate program termination
 */
int request_error_handler(request_status_t error);


/***************| Additional Helpers |***************/
/**
 * @brief Converts 32-bit integer to binary string representation
 * 
 * @param binary Pointer to binary string
 * @param hex Hex value of number to be converted
 */
void itob(char* binary, unsigned int hex);

/**
 * @brief Converts binary string to integer value
 * 
 * @param binary Null-terminated binary string to convert
 * @return int Decimal representation of binary input
 */
unsigned int btoi(const char* binary);

/**
 * @brief Checks if value is a power of two
 * 
 * @param n Value to check
 * @return true 
 * @return false 
 */
bool isPowerOfTwo(int n);


#endif // CACHE_H

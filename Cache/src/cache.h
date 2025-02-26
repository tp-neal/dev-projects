
#ifndef CACHE_H
#define CACHE_H

/***************************************************************************************************
* @project: Direct-Mapped Write-Back Cache [Trace Driven Simulation]
****************************************************************************************************
* @file cache.h
* @brief Contains structures and function declarations related to the sumulation 
         logic.
*
* @author Tyler Neal
* @date 2/26/2025
***************************************************************************************************/

#include <stdlib.h>
#include <stdbool.h>

#include "config.h"
#include "error.h"

/*==================================================================================================
    Cache/Request Structures/Enums
==================================================================================================*/

/***************| Types |***************/
typedef enum {
	UNIFIED = 'U', // both Data and Instruction
	DATA = 'D',
	INSTRUCTION = 'I'
} reference_type_e;

typedef enum {
	READ = 'R',
	WRITE = 'W',
} access_type_e;

/***************| Cache |***************/
typedef struct {
  bool dirty; // represents the dirty bit
  char tag[INSTRUCTION_SIZE];
} line_s;

typedef struct {
  // Cache Details
  size_t layer;         // (e.g L1, L2, L3)
  size_t cache_size;    // (bytes)
  size_t line_size;     // (bytes)
  size_t num_lines;     // (lines/block)
  line_s *lines;

  // Feild Sizes
  unsigned int tag_size;
  unsigned int index_size;
  unsigned int offset_size;

  // Recorded Metrics
  size_t requests;
  size_t hits;
  size_t misses;
  size_t read_to_write;     // reads resulting in write-backs
  size_t write_to_write;    // writes resulting in write-backs
} cache_s;

/***************| Request |***************/
typedef struct { 
  unsigned int hex;
  char binary[33]; // temp size

  char tag[INSTRUCTION_SIZE+1]; // +1 for null terminator
  unsigned int tag_size;

  char index[INSTRUCTION_SIZE+1];
  unsigned int index_size;

  char offset[INSTRUCTION_SIZE+1];
  unsigned int offset_size;

} address_s;

typedef struct {
  reference_type_e ref_type;
  access_type_e access_type;
  address_s address;
} request_s;

/*==================================================================================================
    Environment Structures
==================================================================================================*/

typedef struct {
    cache_s* cache[3];
    size_t cache_layers;
    size_t layer_sizes[3];
    reference_type_e cache_type;
    size_t line_size;
    unsigned int print_style;
} environment_info_s;

/*==================================================================================================
    Simulation Function Declarations
==================================================================================================*/

/***************| Parameter Handling |***************/

error_status_s retrieveParameters(environment_info_s* env_info, int argc, char** argv);


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
error_status_s allocateCache(cache_s** cache, size_t layer, size_t cache_size, size_t line_size);

void destroyCache(cache_s* cache);

error_status_s setupCache(cache_s** cache, size_t layer, size_t cache_size, size_t line_size);


/***************| Request |***************/

error_status_s allocateRequest(request_s** request);

void destroyRequest(request_s* request);

error_status_s formatRequest(request_s* request, cache_s* cache, const char* buffer);

error_status_s formatRequestAddressFields(request_s* request, const char* buffer);

error_status_s processRequest(request_s* request, cache_s* cache, bool* hit_occured);


/***************| Printing |***************/

error_status_s printResults(environment_info_s env);

error_status_s printCacheStats(cache_s* cache, unsigned int print_style);

void printAMAT(cache_s** cache, size_t layers);


/***************| Additional Helpers |***************/

void hexToBinaryString(char* binary, unsigned int hex);

unsigned int binaryStringToInt(const char* binary);

bool isPowerOfTwo(int n);


#endif // CACHE_H

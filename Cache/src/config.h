
#ifndef CONFIG_H
#define CONFIG_H

/***************************************************************************************************
* @project: Direct-Mapped Write-Back Cache [Trace Driven Simulation]
****************************************************************************************************
* @file config.h
* @brief Contains macros relating to cache configurations.
*
* @author Tyler Neal
* @date 2/26/2025
***************************************************************************************************/

/*==================================================================================================
    Macros
==================================================================================================*/

// Used for debug printing
#define DEBUG false

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
#define TRACE_SIZE 11

#endif // CONFIG_H
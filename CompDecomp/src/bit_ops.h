
/*
===========================================================================
 PROJECT: Compression and Decompression Algorithm
===========================================================================
 NAME: Tyler Neal
 DATE: 02/19/2025
 FILE NAME: bit_ops.h
 DESCRIPTION:
    This file contains function definitions for bit manipulation, as well
    as additional macros.
===========================================================================
*/

#ifndef BIT_OPS_H
#define BIT_OPS_H

/* ===================================================================
                                Macros
=================================================================== */

#define EOF_SIGNAL 256
#define INPUT_BUFFER_SIZE 1024
#define OUTPUT_BUFFER_SIZE 1024

#define CHECK_WRITE(write_call)             \
        do {                                \
            if (write_call < 0) {           \
                return ERR_WRITE_FAILURE;   \
            }                               \
        } while (0);                        \

#define CHECK_READ(read_call)               \
        do {                                \
            if (read_call < 0) {            \
                return ERR_READ_FAILURE;    \
            }                               \
        } while (0);                        \

/* ===================================================================
                            Error Handling
=================================================================== */

typedef enum {
    SUCCESS = 0,
    ERR_WRITE_FAILURE = -1,
    ERR_READ_FAILURE = -2,
    ERR_INDEX_OUT_OF_BOUNDS = -3
} error;

/* ===================================================================
                         Function Declarations
=================================================================== */

 /**
  * @brief Read the next bit from STDOUT into a buffer
  * 
  * @param byte Pointer to bit buffer to be loaded
  *             Valid character value: 0-255
  *             End of file signal: 256
  * @return int 0 on success : negative on failure
  */
int read_bit(unsigned short* bit);

 /**
  * @brief Read the next byte from STDOUT into a buffer
  * 
  * @param byte Pointer to byte buffer to be loaded
  *             Valid character value: 0-255
  *             End of file signal: 256
  * @return int 0 on success : negative on failure
  */
int read_byte(unsigned short* byte);

/**
 * @brief Load a bit into the 8-bit write buffer. And write the buffer
 * if it is full.
 * 
 * @param bit Bit to be loaded into buffer
 */
int write_bit(unsigned char bit);

/**
 * @brief Write an entire byte to STDOUT
 * 
 * @param byte To be written to STDOUT
 */
int write_byte(unsigned char byte);

/**
 * @brief Flush any remaining bits to STDOUT
 * 
 */
int flush_write_buffer();


#endif // BIT_OPS_H
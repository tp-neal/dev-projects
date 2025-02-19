
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
            if (write_call == -1) {         \
                return ERR_WRITE_FAILURE;   \
            }                               \
        } while (0);                        \

/* ===================================================================
                            Enumerators
=================================================================== */

typedef enum {
    SUCCESS = 0,
    ERR_WRITE_FAILURE = -1,
    ERR_INDEX_OUT_OF_BOUNDS = -2
} error;

/* ===================================================================
                         Function Declarations
=================================================================== */

unsigned short read_byte();

unsigned short read_bit();

int write_byte(unsigned char byt);

int write_bit(unsigned char bit);

int flush_write_buffer();


#endif // BIT_OPS_H
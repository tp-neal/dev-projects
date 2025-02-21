
/*
===========================================================================
 PROJECT: Compression and Decompression Algorithm
===========================================================================
 NAME: Tyler Neal
 DATE: 02/19/2025
 FILE NAME: bit_ops.c
 DESCRIPTION:
    This file contains function implementations for bit mainpulation.
    The read_bit() and read_byte() functions return an unsigned short,
    instead of an unsigned character in order to properly signal end of
    file. As an unsigned char is limited to the range of value 0-255 we
    assign EOF_SIGNAL the value of 256 so that the signal does not share
    the value of a valid character.
===========================================================================
*/

#include <unistd.h>
#include "bit_ops.h"

static char read_buffer = 0;
static int read_index = -1; // -1 

static char write_buffer = 0;
static int write_index = 7; // n-1 of bytesize

/* ===================================================================
                          Reading Functions
=================================================================== */

 /**
  * @brief Read the next bit from STDOUT into a buffer
  * 
  * @param byte Pointer to bit buffer to be loaded
  *             Valid character value: 0-255
  *             End of file signal: 256
  * @return int 0 on success : negative on failure
  */
int read_bit(unsigned short* bit) {

    // Check if previous byte is done being read
    if (read_index < 0) {

        // Read next byte and reset byte index
        ssize_t bytes_read = read(STDIN_FILENO, &read_buffer, 1);

        if (bytes_read == -1) {
            return ERR_READ_FAILURE;
        }

        // Check for end of file.
        if (bytes_read == 0) {
            *bit =  EOF_SIGNAL;
            return SUCCESS;
        }

        read_index = 7; // move index to read bytes MSB
    }

    // Grab next bit from buffered byte from left -> right (MSB -> LSB)
    *bit = ((read_buffer >> read_index--) & 1);

    return SUCCESS;
}

 /**
  * @brief Read the next byte from STDOUT into a buffer
  * 
  * @param byte Pointer to byte buffer to be loaded
  *             Valid character value: 0-255
  *             End of file signal: 256
  * @return int 0 on success : negative on failure
  */
int read_byte(unsigned short* byte) {
    unsigned short bit = 0;

    // Iterate each bit in byte
    for (int i = 7; i >= 0; i--) {
        CHECK_READ(read_bit(&bit));

        // Check for end of file.
        if (bit == EOF_SIGNAL) {
            *byte =  EOF_SIGNAL;
            return SUCCESS;
        }

        // If bit is 1, XOR shift it, and simply skip 0's
        if (bit) {
            *byte |= (1 << i);
        }
    }

    return SUCCESS;
}


/* ===================================================================
                          Writing Functions
=================================================================== */

/**
 * @brief Load a bit into the 8-bit write buffer. And write the buffer
 * if it is full.
 * 
 * @param bit Bit to be loaded into buffer
 */
int write_bit(unsigned char bit) {
    // Load the bit into the 8-bit write buffer
    if (bit) {
        write_buffer |= (1 << write_index);
    }

    // Shift bit pointer in write buffer
    write_index--;

    // If byte buffer is full, write it out.
    if (write_index < 0) {
        ssize_t bytes_wrote = write(STDOUT_FILENO, &write_buffer, 1);
        
        // Pass up write error
        if (bytes_wrote == -1) {
            return ERR_WRITE_FAILURE;
        }
        
        write_buffer = 0; // zero out buffer
        write_index = 7; // reset index to point to MSB
    }

    return SUCCESS;
}

/**
 * @brief Write an entire byte to STDOUT
 * 
 * @param byte To be written to STDOUT
 */
int write_byte(unsigned char byte) {
    unsigned char bit;
    int status; // holds return status of write op

    // Write each bit to STDOUT from left to right
    for (int i = 7; i >= 0; i--) {
        bit = (byte >> i) & 1;
        status = write_bit(bit);
        
        // Check for write error
        if(status == ERR_WRITE_FAILURE) {
            return ERR_WRITE_FAILURE;
        }
    }

    return SUCCESS;
}

/**
 * @brief Flush any remaining bits to STDOUT
 * 
 */
int flush_write_buffer() {
    unsigned int empty_bits = (7 - write_index);
    int status; // holds return status of write op

    // Flush the remaining bits from the buffer
    for (int i = 0; i < empty_bits; i++) {
        status = write_bit(1);
        
        // Check for write error
        if(status == ERR_WRITE_FAILURE) {
            return ERR_WRITE_FAILURE;
        }
    }

    return SUCCESS;
}

/*
===========================================================================
 PROJECT: Compression and Decompression Algorithm
===========================================================================
 NAME: Tyler Neal
 DATE: 02/19/2025
 FILE NAME: compress.c
 DESCRIPTION:
    Main function for the dzy de-compression implementation.
  
    This program decompresses a compressed stream directed at its standard input 
    and writes the decompressed data to its standard output.
===========================================================================
*/

#include <stdbool.h>
#include "bit_ops.h"


int main(int argc, char *argv[]) {

    // Array of previously seen characters.
    char previous[8] = {0};
    unsigned short bit = 0;
    unsigned short byte = 0;

    // Continue processing until EOF.
    while (true) {

        // Read in next bit.
        bit = read_bit();

        // Break at end of input stream
        if (bit == EOF_SIGNAL) {
            break;
        }

        // Bit indicates non-duplicate value
        if (bit == 1) {

            byte = read_byte();

            // Check again for end of stream
            if (byte == EOF_SIGNAL) {
                break;
            }

            CHECK_WRITE(write_byte(byte));

        // Bit indicates duplicate value    
        } else if (bit == 0) {

            // Read the next 3 bits to determine the index of the previous byte.
            unsigned int buffer[3];
            for (unsigned int i = 0; i < 3; i++) {
                buffer[i] = read_bit();
            }

            // Convert the 3 bits to an integer index.
            int index = ((buffer[0] << 2) | (buffer[1] << 1) | buffer[2]);

            // Check the index bounds
            if (index > 7 || index < 0) {
                return ERR_INDEX_OUT_OF_BOUNDS;
            }

            // Retrieve the byte from the previously seen characters.
            byte = previous[index];

            // Write the byte to the output.
            CHECK_WRITE(write_byte(byte));
        }

        // Update the list of previously seen bytes.
        for (unsigned int i = 7; i > 0; i--) {
            previous[i] = previous[i - 1];
        }
        previous[0] = byte;
    }

    // Flush any remaining data in the write buffer.
    CHECK_WRITE(flush_write_buffer());

    // Exit successfully.
    return SUCCESS;
}

/*
===========================================================================
 PROJECT: Compression and Decompression Algorithm
===========================================================================
 NAME: Tyler Neal
 DATE: 02/20/2025
 FILE NAME: compress.c
 DESCRIPTION:
    This program compresses the data stream directed at its standard input 
    and writes the compressed stream to its standard output.
   
    The compression algorithm reads the input one symbol (i.e., byte) at a 
    time and compares it with each of the 8 bytes previously seen. It also 
    checks to see if the following n characters are the same as the current 
    symbol. If the byte has been seen before, it outputs the position of 
    the previous byte relative to the current position. Otherwise, the symbol 
    is output as is, prefixed with a binary one.
===========================================================================
*/

#include <stdbool.h>
#include "bit_ops.h"


int main(int argc, char *argv[]) {

    // Array of previously seen characters.
    char previous[8] = {0};
    unsigned short byte = 0;

    // Continue processing until EOF.
    while (true) {

        // Read the next byte.
        byte = 0;
        CHECK_READ(read_byte(&byte));

        // Break at end of output stream
        if (byte == EOF_SIGNAL) {
            break;
        }

        // Compare with the previous eight bytes.
        bool is_duplicate = false;
        for (int i = 0; i < 8; i++) {

            // Skip until matchi is found
            if (byte != previous[i]) {
                continue;
            }

            // Write a 0 bit followed by a 3-bit offset.
            CHECK_WRITE(write_bit(0));
            for (int j = 2; j >= 0; j--) {
                write_bit((i >> j) & 1);
            }

            is_duplicate = true;
            break; // Exit the loop if a match is found.
        }

        // If no match was found.
        if (!is_duplicate) {
            CHECK_WRITE(write_bit(1));
            CHECK_WRITE(write_byte(byte));
        }

        // Update the list of previously seen bytes.
        for (int i = 7; i > 0; i--) {
            previous[i] = previous[i - 1];
        }
        previous[0] = byte;
    }

    // Flush any remaining data in the write buffer.
    CHECK_WRITE(flush_write_buffer());

    // Exit successfully.
    return SUCCESS;
}
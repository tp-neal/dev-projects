# Project: Compression and Decompression

## Overview:

This project implements a data compression system consisting of two main components:

Compress - An executable that applies a custom compression algorithm to data streams. It reads from standard input and writes the compressed output to standard output.
Decompress - A complementary executable that reverses the compression process. It takes compressed data from standard input and outputs the original, uncompressed data to standard output.

## Building:

### Prerequisites:

- GCC compiler
- Make build system
- Bash shell (for running tests)

### Makefile:

To build the makefile:
- Navigate to "src/"
- Run
```bash
make all 
``` 

## Usage:

To compress:
```bash
./compress < input_file > compressed_file
```

To decompress:
```bash
./decompress < compressed_file > decompressed_file
```

### Example:
```bash
./compress < compress.c > compress.c.czy
./decompress < compress.c.czy > copy_compress.c
```

## Testing:

- Naviate to "tests/"
- Run the test script:
```bash
# Script will recompile the source code upon invokation
./run.sh
```

This will compress the source code and executable using the compression algorithm
and gzip, and then compare the resulting sizes.

## Compression Results:

Note: Compression ratios are calculated as `(compressed_size / original_size) * 100`.

### Original Sizes:

- `compress.c`: 2381 bytes
- `compress`: 16408 bytes

### Using 'Compression':

- `compress.c.czy`: 1964 bytes (Compression ratio: 82.49%)
- `compress.obj.czy`: 10070 bytes (Compression ratio: 61.37%)

### Using 'Gzip':

- `compress.c.gz`: 991 bytes (Compression ratio: 41.62%)
- `compress.obj.gz`: 3154 bytes (Compression ratio: 19.22%)

## Author:

Tyler Neal


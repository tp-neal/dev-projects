# Project: Compression and Decompression

## Overview:

This project implements a loseless data compression algorithm using Hoffman Encoding.

The project consists of two main components:

1. Compress - An executable that applies a custom compression algorithm to data streams. It reads from standard input and writes the compressed output to standard output.
2. Decompress - A complementary executable that reverses the compression process. It takes compressed data from standard input and outputs the original, uncompressed data to standard output.

## Building:

### Prerequisites:

-   GCC compiler
-   Make build system
-   Bash shell (for running tests)

### Makefile:

To build the makefile:

-   Navigate to "src/"
-   Run

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

-   Naviate to "tests/"
-   Run a test script:

run1.sh:

```bash
# Script will recompile the source code upon invokation
./run1.sh
```

This will compress the source code and executable using the compression algorithm
and gzip, and then compare the resulting sizes.

run2.sh:

```bash
# Script will recompile the source code upon invokation
./run2.sh
```

This will compress the canterbury benchmark files, and determine the compression ratio from the
original to the compressed versions.

At the end it will determine the average compression ratio among all benchmarks.

## Compression Results:

Note: Compression ratios are calculated as `(compressed_size / original_size) * 100`.

## Author:

Tyler Neal

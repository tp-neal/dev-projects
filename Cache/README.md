# Project: Cache Simulator

## Overview:

A trace-driven cache simulator implementing a direct-mapped write-back cache hierarchy with configurable L1, L2, and L3 caches.
Designed to test the SPEC95 benchmarks.

## Features:

- Supports up to 3-level cache hierarchy
- Configurable cache and line sizes
- Direct-mapped architecture with write-back policy
- Separate instruction and data cache simulation
- Detailed performance metrics including:
  - Hit/Miss rates
  - Average Memory Access Time (AMAT)
  - Read-to-Write and Write-to-Write ratios

## Supported Benchmarks:

- 099.go
- 124.m88ksim
- 126.gcc
- 129.compress
- 132.ijpeg
- 134.perl

### *NOTICE!*

The benchmarks are too large to be contained within the repository. Please follow the below link to download these files.
```
https://drive.google.com/drive/folders/14rs7B_XDDwIt9W74svfScrMwoWW0kEBe?usp=sharing
```
Once downloaded make sure the are contained in a folder named "traces" within the project's root directory

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

```bash
./cache_exec <cache_type> <line_size> <cache_layers> <L1_size> <L2_size> <L3_size> <print_style>
```

### Parameters:

- `cache_type`: Cache type to simulate (I=Instruction, D=Data, U=Unified)
- `line_size`: Size of cache line in words (1 word = 4 bytes)
- `cache_layers`: Number of cache layers (1-3)
- `L1_size`: L1 cache size in KB
- `L2_size`: L2 cache size in KB (0 if unused)
- `L3_size`: L3 cache size in KB (0 if unused)
- `print_style`: Output format (1=Basic, 2=Detailed)

### Example:

```bash
# Single layer 8KB L1 cache with 4-word lines
./cache_exec U 4 1 8 0 0 1 < ../traces/126.gcc

# Three layer cache hierarchy of (L1:16KB, L2:64KB, L3:256KB) with detailed output
./cache_exec D 8 3 16 64 256 2 < ../traces/132.ijpeg
```

## Testing:

Run the full test suite:
```bash
# Both scripts will recompile the source code upon invokation
./run1.sh
./run2.sh
```

This will execute various cache configurations using the provided trace files.

## Architecture:

The simulator implements a direct-mapped cache with the following features:
- Write-back policy for handling writes
- 32-bit addressing
- Configurable tag, index, and offset bits based on cache parameters
- Support for both unified and split I/D caches

## Performance:

Memory access times are modeled as follows:
- L1 Cache: 1 cycle
- L2 Cache: 16 cycles
- L3 Cache: 64 cycles
- Main Memory: 100 cycles

AMAT (Average Memory Access Time) is calculated based on hit rates at each level.

## Author:

Tyler Neal
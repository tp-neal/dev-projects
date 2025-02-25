# Dev Portfolio

This repository contains several projects that best demonstrate my ability to write clean, modular, and robust code for low level managment.

## Projects

### Project 1: RPC System Calls *(RPCSysCalls)*

This project is a client-server system that allows clients to preform system calls on a remote file system. The server is able to handle multiple clients, and send them back the results of the system calls.

### Project 2: Direct-Mapped Write-Back Cache Trace-Based Simulation *(Cache)*

Gathers statistics on a direct-mapped write-back cache with varying layer counts {1..3} through traces that simulate memory accesses. Used to provide insight into the efficiency of different configurations in regards to average memory access times. Configurable cache layer count, layer sizes, as well as line sizes.

### Project 3: Command Line Logging *(CommandLog)*

Program used to execute a given bash command, log its input, output, and errors to respective files, and display the command's output and errors in real-time to the terminal.

### Project 4: Compression and Decompression *(CompDecomp)*

Compression and corresponding decompression algorithm that use redudancies in byte data in order to compress and decompress from stdin to stdout.

### Project 5: Santa-Multi-Thread *('')* **[FIX IN PROGRESS!]**

Utilizes multithreading and primitive synchornization techniques to simulate Santa's workshop, and present delivery process. A Hoare-Style monitor is used in order to synchronize threads representing Santa, his elves, and reindeer. Allows for configurable elv and reindeer thread counts, as well as number of deliveries made before jprogram termination.

## Tools

- C
- C++
- Bash
- Makefile

## Required Fixes

Santa-Multi-Thread:
- My original implementation of this program used a proprietary thread library. This library included the Hoare-Style monitor class, which is not included in the standard C++ thread library. Because of this, I've decided to rebuild the project using the standard C++ thread library, instead implementing a Mesa-Style monitor, as this style is more supported by the way that the standard C++ thread library functions. Most of the code is already fixed, but there is still an issue to be fixed with thread initialization, which I plan to work on soon.

## Contact
- GitHub: [tp-neal] https://github.com/tp-neal
- LinkedIn: https://www.linkedin.com/in/tyler-neal-dev
- Email: tylerneal.dev@gmail.com
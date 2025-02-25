# Project: RPC System Calls

## Overview:

This project is a client-server system that allows clients to preform system calls on a remote file system. The file system can perform the following operations on files in the remote server: 
- open() 
- close() 
- read() 
- write() 
- lseek()
- checksum()

The program uses sockets to perform network communication, and ensures byte order conversions before data transfer / after recieving.

## Constraints:

- Basic file operation
- Integers of up to 32 bits to be transfered
- XOR Computed checksums

## Features:

- Remote file operations
- Robust error handling
- Network byte order conversion for cross-platform compatibility
- File integrity verification via checksums
- Checksum block size control
- Simultaneous client handling via forks
- Proper heap memory managment

## Building:

### Prerequisites:

- GCC compiler
- Make build system
- POSIX-compliant OS

### Makefile:

To build the project:
- Navigate to "src/"
- Run

```bash
make all 
``` 

## Usage:

Server:
```bash
./server <port>
```

Client:
```bash
./user <hostname> <port> <remote_file_path> <local_file_path>
```

### Parameters:

- `hostname`: IPv4 address of the server
- `port`: Port number to connect to the server
- `remote_file_path`: Path to the file on the remote server
- `local_file_path`: Path where the file should be saved locally

### Example:

```bash
# Start the server on port 8080
./server 8080

# In another terminal, run the client
./user 127.0.0.1 8080 remote.md local_copy.md
```

## Testing:

To run the standard test:
- Navigate to the "tests" directory
- Run the following commands in seperate terminals
```bash
# Run this in a terminal
./run_server

# In another terminal, run the client
./run_user
```
**Note** This test defaults the port number to 9000, so make sure its not in use, or specify another port in both scripts $PORT variable

The project includes a simple validation mechanism:

The client requests a checksum of the remote file
After copying, it computes a checksum of the local file
It then compares the checksums to verify file integrity

Example output on successful transfer:
```
[User : Info] Remote checksum: 123
[User : Info] Local checksum: 123
[User : Info] SUCCESS: File copied successfully (Checksums match: 123)
```

## Author:

Tyler Neal
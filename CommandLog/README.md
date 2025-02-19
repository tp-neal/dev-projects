# Project: Command Line Logging

## Overview:

This program provides functionality for managing multiple file
descriptors, creating files and directories, and piping data
between processes. Its primary use-case is to execute a given
command, log its input, output, and errors to respective files,
and display the command's output and errors in real-time to the
terminal.

The program ensures proper cleanup of opened file descriptors
in error scenarios and executes the given command using a
child process.

## Constraints:

- The command given must be valid, else the program will simply output
  error information.
- The log directory will be generated in the source folder.

## Features:

- Command line log filing
- Support for multiple command switches
- File descriptor management and cleanup
- Clean organized console output.

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
usage: ./hscript <command> <arguments> ... <log_directory_name>
```

### Parameters:

- `command`: Bash command to be ran
- `arguments`: Switches provided to the command (may use multiple)
- `log_directory_name`: Name to write the log files too *(note: the makefile assumes the log directory name of 'dir', and a program error log of 'err_log' upon cleanup)*

### Example:
```bash
./hscript ls -l -a dir
```
## Testing:

- Naviate to "tests/"
- Run the test script:
```bash
# Script will recompile the source code upon invokation
./run.sh
```

This will run the script with the arguments ls -l -a, and log its output to the respective files

## Author:

Tyler Neal

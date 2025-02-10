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

## Required:

1. C++ compiler
2. Bash shell

## Usage:

```usage: ./hscript [command] [arguments] ... [log_directory_name]```

**Simple:**
*(default parameters)*

1. Navigate to "/src/"
2. Run the file "run.sh" 
    
**Custom:**
*(modified parameters)*

1. Navigate to "/src/"
2. Make clean
3. Make all
4. Run the program with optional parameters
**(note: the makefile assumes the log directory name of 'dir', and a program error log of 'err_log' upon cleanup)**

# Project: Command Line Logging

## Overview:

This program takes a command typed in the terminal, and logs its ouput
and error information to a specified directory. The output and error 
information is also then displayed to the terminal, as if it weren't
intercepted.

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
**(note: the makefile assumes the log directory name of 'log' upon cleanup)**

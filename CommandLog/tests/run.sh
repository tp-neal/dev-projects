#!/usr/bin/env bash

# This script is a simple demonstration of the program.
# Feel free to modify the arguments here.

EXEC_DIRECTORY="../src/"

# Clean and build environment
cd "$EXEC_DIRECTORY"
make clean
make all
clear

echo ""

./hscript ls -l -a dir

echo ""
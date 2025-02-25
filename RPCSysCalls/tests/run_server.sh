#!/bin/bash

PORT=9000

# Move to makefile directory
cd ../src/

# Clean loose files
make clean
clear

# Build files
echo ""
echo "Building files"
echo "---------------"
make
echo ""

# Run server
./server $PORT
echo ""
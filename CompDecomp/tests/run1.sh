#!/usr/bin/env bash

EXEC_DIRECTORY="../src/"

# Clean and build environment
cd "$EXEC_DIRECTORY"
echo ""
echo "Building Environment"
echo "---------------------"
make clean
make all

# Compress source code and object file of compression algorithm
./compress < compress.c > compress.c.czy
./compress < compress > compress.obj.czy

# Compress source code and object file using a reknown compression tool
gzip -c compress.c > compress.c.gz
gzip -c compress > compress.obj.gz

# List file size details about original and compressed files
list=("compress.c" "compress" "compress.c.czy" 
      "compress.obj.czy" "compress.c.gz" "compress.obj.gz")

echo ""
echo "Compression Results"
echo "---------------------"
for item in "${list[@]}"; do
    wc -c "$item"
done

echo ""
echo "Script finished."

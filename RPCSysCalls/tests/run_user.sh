#!/bin/bash

HOSTNAME=127.0.0.1
PORT=9000
TEXT_FILE_DIR="../text_files/"

# Move to makefile directory
cd ../src/

# Clear display
clear

# Run user with specified parameters
./user $HOSTNAME $PORT "${TEXT_FILE_DIR}remote.md" "${TEXT_FILE_DIR}local_copy.md"
echo ""

# Print contents of copied file
cat "${TEXT_FILE_DIR}local_copy.md"
echo -e "\n"
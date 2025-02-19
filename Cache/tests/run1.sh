#!/bin/bash

# PART 1 -------------------------------------

# Filepaths
executable_dir='../src/'
source_dir='../src/'
tests_dir='../tests/'
traces_dir='../traces/'

# Clean up and compile environment
echo "Cleaning up environment and compiling..."
cd "${source_dir}"
make clean
make all
clear
cd "${tests_dir}"

# Trace file setup
trace=('126.gcc' '129.compress' '132.ijpeg' '134.perl' '099.go' '124.m88ksim')

# Fixed arguments
cache_layers='1'
empty_layer='0'
print_style='1'

# Array loops
cache_types=('U' 'I' 'D')
cache_sizes=('8' '16')
line_sizes=('4' '8')

echo "Starting cache configuration tests..."

# Concatenate path to executable
executable_path="${executable_dir}cache_exec"

# Loop over every permutation of trace, cache size, and line size
for trace in "${trace[@]}"; do
  trace_path="${traces_dir}${trace}" # Concatenate the path to the trace file
  echo -e "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
  echo -e "\t\t\t Testing trace $trace..."
  echo -e "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
  for type in "${cache_types[@]}"; do
    echo -e "================================================================\n"
    for size in "${cache_sizes[@]}"; do
      for line in "${line_sizes[@]}"; do
        echo -e "------------------------------------------------------------"
        echo -e "Configuration: Type $type, Cache Size ${size}kb, Line Size ${line} words"
        echo -e "------------------------------------------------------------"
        ./"${executable_path}" $type $line $cache_layers $size $empty_layer $empty_layer $print_style < $trace_path
        echo -e "------------------------------------------------------------\n"
      done
    done
  done
done

echo "============================================================"
echo -e "\n\t\t\t ALL FINISHED! \n"
echo -e "============================================================\n"

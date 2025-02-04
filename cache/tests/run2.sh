#!/bin/bash

#===========================================================================
# PROJECT: Direct-Mapped Write-Back Cache [Trace Driven Simulation]
#===========================================================================
# NAME : Tyler Neal
# DATE: 01/09/2025
# FILE NAME : run2.sh
# DESCRIPTION:
#    This script performs a test on the cache under the following assumptions:
#      1. Only data references will be tested
#      2. Line size is always 8 words (32 bytes)
# 
# Note:
#   The script will perform tests under every combination of the following 
#   variables:
#      Number of Layers:   1, 2, 3
#      L1 Cache Size:      4KB, 16KB
#      L2 Cache Size:      32KB, 64KB
#      L3 Cache Size:      256KB, 1024KB
#===========================================================================

# PART 2 -------------------------------------

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

# Fixed arguments
cache_type='D'
line_size='8'
print_style='2'

# Trace file setup
trace=('126.gcc' '129.compress' '132.ijpeg' '134.perl' '099.go' '124.m88ksim')

# Layer counts
layers=('1' '2' '3')

# Cache sizes
null_cache='0'
L1_sizes=('4' '16')
L2_sizes=('32' '64')
L3_sizes=('256' '1024')

# Counter for configuration number
config_count=0
config_count_1=0

echo "Starting cache configuration tests..."

# Concatenate path to executable
executable_path="${executable_dir}cache_exec"

# Loop over every permutation of layer, cache size as needed
for trace in "${trace[@]}"; do
    config_count=0
    config_count_1=$((config_count_1 + 1))
    trace_path="${traces_dir}${trace}"  # Concatenate the path to the trace file
    echo -e "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
    echo -e "\t\t Testing trace $trace..."
    echo -e "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
    for layer in "${layers[@]}"; do
        echo -e "\n====================================================================="
        echo "Testing configurations for Layer $layer..."
        echo -e "=====================================================================\n"
        if [[ "$layer" == "1" ]]; then
            for L1_size in "${L1_sizes[@]}"; do
                config_count=$((config_count + 1))
                echo -e "------------------------------------------------------------"
                echo -e "Configuration[$config_count_1-$config_count]: Layers $layer, L1 size ${L1_size}kb"
                echo -e "------------------------------------------------------------"
                ./"${executable_path}" $cache_type $line_size $layer $L1_size $null_cache $null_cache $print_style < $trace_path
                echo -e "------------------------------------------------------------\n"
            done
        elif [[ "$layer" == "2" ]]; then
            for L1_size in "${L1_sizes[@]}"; do
                for L2_size in "${L2_sizes[@]}"; do
                    config_count=$((config_count + 1))
                    echo -e "------------------------------------------------------------"
                    echo -e "Configuration[$config_count_1-$config_count]: Layers $layer, L1 size ${L1_size}kb, L2 size ${L2_size}kb"
                    echo -e "------------------------------------------------------------"
                    ./"${executable_path}" $cache_type $line_size $layer $L1_size $L2_size $null_cache $print_style < $trace_path
                    echo -e "------------------------------------------------------------\n"
                done
            done
        elif [[ "$layer" == "3" ]]; then
            for L1_size in "${L1_sizes[@]}"; do
                for L2_size in "${L2_sizes[@]}"; do
                    for L3_size in "${L3_sizes[@]}"; do
                        config_count=$((config_count + 1))
                        echo -e "------------------------------------------------------------"
                        echo -e "Configuration[$config_count_1-$config_count]: Layers $layer, L1 size ${L1_size}kb, L2 size ${L2_size}kb, L3 size ${L3_size}kb"
                        echo -e "------------------------------------------------------------"
                        ./"${executable_path}" $cache_type $line_size $layer $L1_size $L2_size $L3_size $print_style < $trace_path
                        echo -e "------------------------------------------------------------\n"
                    done
                done
            done
        fi
    done
done

echo "============================================================"
echo -e "\n\t\t\t ALL FINISHED! \n"
echo -e "============================================================\n"

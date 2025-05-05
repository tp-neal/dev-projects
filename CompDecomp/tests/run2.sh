#!/usr/bin/env bash

EXEC_DIRECTORY="../src"
TEST_FILES_DIRECTORY="../canterbury"

cd "$EXEC_DIRECTORY" || exit 1

echo ""
echo "Building Environment in $(pwd)"
echo "---------------------"
make clean
if ! make all; then
    echo "Make failed, exiting."
    exit 1
fi
if [[ ! -x "./compress" ]]; then
    echo "Compress executable not found or not executable after make, exiting."
    exit 1
fi

echo ""
echo "Compressing files from $TEST_FILES_DIRECTORY"
echo "---------------------"
for item in "$TEST_FILES_DIRECTORY"/*; do
    if [[ -f "$item" ]]; then
        echo "Compressing '$item'..."
        ./compress < "$item" > "$item.czy"
    fi
done

echo ""
echo "Compression Results"
echo "---------------------"
total_original_size=0
total_compressed_size=0
found_results=0
for item in "$TEST_FILES_DIRECTORY"/*.czy; do
    if [[ -e "$item" ]]; then
        found_results=1
        original_file="${item%.czy}"

        if [[ -f "$original_file" ]]; then
            compressed_size=$(wc -c < "$item")
            original_size=$(wc -c < "$original_file")

            # Add to totals
            total_original_size=$(($total_original_size+$original_size))
            total_compressed_size=$(($total_compressed_size+$compressed_size))

            echo "File: $original_file"
            printf "  Original size:   %d\n" "$original_size"
            printf "  Compressed size: %d\n" "$compressed_size"

            if [[ "$original_size" -gt 0 ]]; then
                ratio=$(echo "scale=2; $compressed_size * 100 / $original_size" | bc -l)
                printf "  Compression Ratio: %s%%\n" "$ratio"
            else
                printf "  Compression Ratio: N/A (original size is 0)\n"
            fi
            echo "---------------------"
        else
            echo "Original file '$original_file' not found for '$item'."
            echo "---------------------"
        fi
    fi
done

echo "---------------------"
echo "Averages:"
total_compression_ratio=$(echo "scale=2; $total_compressed_size * 100 / $total_original_size" | bc -l)
echo "Total original size: $total_original_size"
echo "Total compressed size: $total_compressed_size"
echo "Total compression ratio: $total_compression_ratio"
echo "---------------------"

if [[ "$found_results" -eq 0 ]]; then
  echo "No .czy files found in $TEST_FILES_DIRECTORY to list."
fi

echo ""
echo "Script finished."
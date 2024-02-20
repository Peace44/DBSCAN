#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")

# Get the paths of the program & the exe
PROG_CPP=$SCRIPT_DIR/basic_dbscan.cpp
PROG=$SCRIPT_DIR/basic_dbscan

# Compile the basic_dbscan.cpp program
g++ -O3 $PROG_CPP -o $PROG -std=c++17

run() {
    dataset_name=$1
    output_file=$2

    # Start time in millisecs
    start=$(date +%s%3N)

    # Run the program and capture the output
    $PROG $dataset_name >> $output_file

    # End time in millisecs
    end=$(date +%s%3N)

    # Calculate exec time
    exec_time=$((end - start))

    # Append exec time to the output file
    echo "Execution time for dataset $dataset_name: $exec_time ms" >> $output_file
}

# Output file
output_file="$SCRIPT_DIR/basic_dbscan.txt"

# Ensure the output file is empty
> $output_file

# Automatically include all files from the INPUTS directory
input_dir="$SCRIPT_DIR/../INPUTS"
input_files=($input_dir/*)

# Run benchmarks for each dataset
for input_file in "${input_files[@]}"; do
    run $input_file $output_file
done

echo "Check the '$output_file' file!"

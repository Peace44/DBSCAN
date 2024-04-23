#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")

# Get the paths of the program & the exe

PROG=$SCRIPT_DIR/mlpack_dbscan



run() {
    dataset_name=$1
    output_file=$2

    # Start time in millisecs
    start=$(date +%s%3N)

    # Run the program and capture the output
    $PROG $dataset_name | tee -a $output_file

    # End time in millisecs
    end=$(date +%s%3N)

    # Calculate exec time
    exec_time=$((end - start))

    # Append exec time to the output file
    echo "Execution time for dataset $dataset_name: $exec_time ms" | tee -a $output_file
}

# Output file
output_file="$SCRIPT_DIR/mlpack_dbscan.txt"
input_file="$SCRIPT_DIR/../INPUTS/random_points.csv"

# Ensure the output file is empty
> $output_file

# Run benchmarks
run $input_file $output_file

echo "built $PROG_CPP Check the '$output_file' file!"


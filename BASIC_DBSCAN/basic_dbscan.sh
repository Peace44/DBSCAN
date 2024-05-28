#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR=$(dirname $(realpath ${BASH_SOURCE[0]}))

# Get the paths of the program & the exe
PROG_CPP=$SCRIPT_DIR/basic_dbscan_opt.cpp
PROG=$SCRIPT_DIR/basic_dbscan

# Compile the program
g++ -O3 $PROG_CPP -o $PROG -std=c++17
if [[ $? -ne 0 ]]; then
    echo "Compilation failed."
    exit 1
fi

run() {
    dataset_name=$1
    output_file=$2
    eps=$3
    minPts=$4
    
    # Start time in milliseconds
    start=$(date +%s%3N)

    # Run the program and capture the output
    $PROG $dataset_name $eps $minPts | tee -a $output_file

    # End time in milliseconds
    end=$(date +%s%3N)

    # Calculate exec time
    exec_time=$((end - start))

    # Append exec time to the output file
    echo "Total execution time for dataset $dataset_name: $exec_time ms" | tee -a "$output_file"
}

# Ensure the correct number of arguments are provided
if [[ $# -ne 3 ]]; then
    echo "Usage: $0 <input_file> <eps> <minPts>"
    exit 1
fi

output_file=$SCRIPT_DIR/basic_dbscan.txt
input_file=$(realpath "$SCRIPT_DIR/$1")
eps=$2
minPts=$3

# Ensure the output file is empty
> $output_file

# Run benchmarks
run $input_file $output_file $eps $minPts

# echo "Completed. Check the '$output_file' file!"

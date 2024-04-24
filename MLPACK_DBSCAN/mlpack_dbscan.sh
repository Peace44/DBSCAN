#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")

# Get the paths of the program & the exe
MLPACK_CPP=$SCRIPT_DIR/mlpack_dbscan.cpp
PROG=$SCRIPT_DIR/mlpack_dbscan

g++ -O3 -std=c++17 -o $PROG $MLPACK_CPP -larmadillo -lmlpack -fopenmp

run() {
    dataset_name=$1
    output_file=$2
    eps=$3
    minPts=$4

    # Start time in millisecs
    start=$(date +%s%3N)

    # Run the program and capture the output
    $PROG $dataset_name $eps $minPts | tee -a $output_file

    # End time in millisecs
    end=$(date +%s%3N)

    # Calculate exec time
    exec_time=$((end - start))

    # Append exec time to the output file
    echo "Execution time for dataset $dataset_name: $exec_time ms" | tee -a $output_file
}

# Output file
output_file="$SCRIPT_DIR/mlpack_dbscan.txt"
#input_file="$SCRIPT_DIR/../INPUTS/random_points.csv"
input_file="$SCRIPT_DIR/"$1
eps=$2
minPts=$3

# Ensure the output file is empty
> $output_file

# Run benchmarks
run $input_file $output_file $eps $minPts

echo "built $PROG_CPP Check the '$output_file' file!"


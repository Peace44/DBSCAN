#!/bin/bash

# Compile the basic_dbscan.cpp program
g++ -O3 basic_dbscan.cpp -o basic_dbscan -std=c++17

run() {
    dataset_name=$1
    output_file=$2

    # Run the program and capture the output
    ./basic_dbscan $dataset_name >> $output_file
}

# Output file
output_file="basic_dbscan.txt"

# Ensure the output file is empty
> $output_file

# Run benchmarks
run "../INPUTS/random_points.csv" $output_file

echo "Check the '$output_file' file!"

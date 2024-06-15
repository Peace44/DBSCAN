#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR=$(dirname $(realpath ${BASH_SOURCE[0]}))

# Get the paths of the program & the exe
MLPACK_CPP=$SCRIPT_DIR/mlpack_dbscan.cpp
PROG=$SCRIPT_DIR/mlpack_dbscan

# Compile the program
g++ -O3 -std=c++17 -o $PROG $MLPACK_CPP -larmadillo -lmlpack -fopenmp
if [[ $? -ne 0 ]]; then
    echo "Compilation failed."
    exit 1
fi

# Ensure the correct number of arguments are provided
if [[ $# -ne 4 ]]; then
    echo "Usage: $0 <input_file> <eps> <min_pts> <norm_type>"
    exit 1
fi

input_file=$(realpath $SCRIPT_DIR/$1)
eps=$2
min_pts=$3
norm_type=$4


# Run benchmarks
echo -ne "\tmicroseconds:\t"
$PROG $input_file $eps $min_pts $norm_type



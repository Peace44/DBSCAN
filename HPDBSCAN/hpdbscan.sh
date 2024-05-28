#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR=$(dirname "$(realpath "${BASH_SOURCE[0]}")")
BUILD_DIR=$SCRIPT_DIR/build

# Compile the program
mkdir $BUILD_DIR && cd $BUILD_DIR && cmake .. && make

# Get the paths of the exe
PROG=$BUILD_DIR/hpdbscan
CSV_TO_HDF5_PROG=$SCRIPT_DIR/csv_to_hdf5.py

run() {
    dataset_name=$1
    output_file=$2
    eps=$3
    minPts=$4

    # Create data.h5
    python3 $CSV_TO_HDF5_PROG $dataset_name $BUILD_DIR/data.h5

    # Start time in milliseconds
    start=$(date +%s%3N)
    
    # Run the program and capture the output
    $PROG -i $BUILD_DIR/data.h5 -o $BUILD_DIR/data.h5 -e $eps -m $minPts | tee -a $output_file
    # $PROG -h | tee -a $output_file

    # End time in milliseconds
    end=$(date +%s%3N)

    # Calculate exec time
    exec_time=$((end - start))

    # Append exec time to the output file
    echo "Total execution time for dataset $dataset_name: $exec_time ms" | tee -a $output_file
}

# Ensure the correct number of arguments are provided
if [[ $# -ne 3 ]]; then
    echo "Usage: $0 <input_file> <eps> <minPts>"
    exit 1
fi

output_file=$SCRIPT_DIR/hpdbscan.txt
input_file=$(realpath $SCRIPT_DIR/$1)
eps=$2
minPts=$3

# Ensure the output file is empty
> $output_file

# Run benchmarks
run $input_file $output_file $eps $minPts

# echo "Completed. Check the '$output_file' file!"

#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR=$(dirname "$(realpath "${BASH_SOURCE[0]}")")
BUILD_DIR=$SCRIPT_DIR/build

# Compile the program
if ! test -d $BUILD_DIR; then
    mkdir $BUILD_DIR && cd $BUILD_DIR && cmake .. && make
fi

# Get the paths of the exe
PROG=$BUILD_DIR/hpdbscan
CSV_TO_HDF5_PROG=$SCRIPT_DIR/csv_to_hdf5.py
HDF5_TO_CSV_PROG=$SCRIPT_DIR/hdf5_to_csv.py

# Ensure the correct number of arguments are provided
if [[ $# -ne 4 ]]; then
    echo "Usage: $0 <input_file> <eps> <min_pts> <norm_type>"
    exit 1
fi

input_file=$(realpath $SCRIPT_DIR/$1)
eps=$2
min_pts=$3
norm_type=$4


# Create data.h5
python3 $CSV_TO_HDF5_PROG $input_file $BUILD_DIR/data.h5

# Run benchmarks
echo -ne "\tmicroseconds:\t"
$PROG -i $BUILD_DIR/data.h5 -o $BUILD_DIR/data.h5 -e $eps -m $min_pts -t 8

# Create hpdbscan.csv
python3 $HDF5_TO_CSV_PROG $BUILD_DIR/data.h5 $SCRIPT_DIR/hpdbscan.csv
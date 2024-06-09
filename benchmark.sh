#!/bin/bash
clear 

exes=("MLPACK_DBSCAN/mlpack_dbscan" "NDUJA_DBSCAN/nduja_dbscan" "BASIC_DBSCAN/basic_dbscan" "BASIC_DBSCAN/basic_dbscan_opt" "KDTREE_DBSCAN/kdtree_dbscan" "KDTREE_DBSCAN/kdtree_dbscan_opt") # "BASIC_DBSCAN++/basic_dbscan++" "HPDBSCAN/hpdbscan" 

# Set default values of DBSCAN parameters: eps and min_pts
eps=$1 #minimize --> current value on nduja 0.35
min_pts=$2 #maximize --> current value on nduja 2
norm_types=("1" "2" "inf") 

# Empty the benchmark txt file or create it if it doesn't exist
benchmark="benchmark_${eps}_${min_pts}.txt"

> $benchmark

echo "Benchmarking with eps = $eps, min_pts = $min_pts" >> $benchmark
echo -e "\n\n" >> $benchmark

for exe in "${exes[@]}"; do
    script="${exe}.sh"
    chmod +x "$script" || echo "Failed to set executable flag on $script"
    ./$script

    echo "Running $exe benchmark..." >> $benchmark
    echo "--------------------------------------------------------------------------------------------------------------------------" >> $benchmark
    for norm_type in ${norm_types[@]}; do
        echo "" >> $benchmark
        python3 benchmark.py --dbscan_program $exe --input_dir INPUTS/CSVs --eps $eps --min_pts $min_pts --norm_type $norm_type >> $benchmark
        echo "" >> $benchmark
    done
    echo "--------------------------------------------------------------------------------------------------------------------------" >> $benchmark
    echo -e "\n\n" >> $benchmark
done

echo "All scripts executed successfully."

#!/bin/bash
clear 

exes=("KDTREE_DBSCAN/kdtree_dbscan_OPT") 
# exes=("BASIC_DBSCAN/basic_dbscan_opt" "BASIC_DBSCAN/basic_dbscan" "NDUJA_DBSCAN/nduja_dbscan" "KDTREE_DBSCAN/kdtree_dbscan_opt" "KDTREE_DBSCAN/kdtree_dbscan" "MLPACK_DBSCAN/mlpack_dbscan") # "BASIC_DBSCAN++/basic_dbscan++" "HPDBSCAN/hpdbscan" 

# Set default values of DBSCAN parameters: eps and min_pts
# eps=$1 #minimize --> current value on nduja 0.35
# min_pts=$2 #maximize --> current value on nduja 2
eps=("0.25" "0.5" "1") 
min_pts=("2" "4" "8") 
norm_types=("1" "2" "inf") 

# Empty the benchmark txt file or create it if it doesn't exist
# benchmark="benchmark_${eps}_${min_pts}.txt"
benchmark="benchmark.txt"

> $benchmark

# echo "Benchmarking with eps = $eps, min_pts = $min_pts" >> $benchmark
# echo -e "\n\n" >> $benchmark

for exe in "${exes[@]}"; do
    script="${exe}.sh"
    chmod +x "$script" || echo "Failed to set executable flag on $script"
    ./$script

    echo "Running $exe benchmark..." >> $benchmark
    echo "--------------------------------------------------------------------------------------------------------------------------" >> $benchmark
    # loop on all epsilon values, min_points and norm_type
    for epsilon in ${eps[@]}; do
        for minPts in ${min_pts[@]}; do
            for norm_type in ${norm_types[@]}; do
        
                echo "" >> $benchmark

                python3 benchmark.py --dbscan_program $exe --input_dir INPUTS/csvs --eps $epsilon --min_pts $minPts --norm_type $norm_type >> $benchmark
                echo "" >> $benchmark
            done
        done
    done
    echo "--------------------------------------------------------------------------------------------------------------------------" >> $benchmark
    echo -e "\n\n" >> $benchmark
done

echo "All scripts executed successfully."

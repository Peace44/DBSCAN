#!/bin/bash
clear 

exes=("BASIC_DBSCAN/basic_dbscan" "BASIC_DBSCAN/basic_dbscan_opt" "KDTREE_DBSCAN/kdtree_dbscan" "KDTREE_DBSCAN/kdtree_dbscan_opt" "MLPACK_DBSCAN/mlpack_dbscan" "NDUJA_DBSCAN/nduja_dbscan") # "BASIC_DBSCAN++/basic_dbscan++" "HPDBSCAN/hpdbscan" 

# Set default values of DBSCAN parameters: eps and min_pts
eps=2.0 #minimize --> current value on nduja 0.35
min_pts=3 #maximize --> current value on nduja 2
norm_types=("1" "2" "inf") 

# Empty the compare.txt file or create it if it doesn't exist
> compare.txt

echo "Benchmarking with eps = $eps, min_pts = $min_pts" >> compare.txt
echo -e "\n\n" >> compare.txt

for exe in "${exes[@]}"; do
    script="${exe}.sh"
    chmod +x "$script" || echo "Failed to set executable flag on $script"
    ./$script

    echo "Running $exe benchmark..." >> compare.txt
    echo "--------------------------------------------------------------------------------------------------------------------------" >> compare.txt
    for norm_type in ${norm_types[@]}; do
        echo "" >> compare.txt
        python3 benchmark.py --dbscan_program $exe --input_dir INPUTS/CSVs --eps $eps --min_pts $min_pts --norm_type $norm_type >> compare.txt
        echo "" >> compare.txt
    done
    echo "--------------------------------------------------------------------------------------------------------------------------" >> compare.txt
    echo -e "\n\n" >> compare.txt
done

echo "All scripts executed successfully."

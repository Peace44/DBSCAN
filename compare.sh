#!/bin/bash
clear 

# Define an array with script names
scripts=("HPDBSCAN/hpdbscan.sh" "NDUJA_DBSCAN/nduja_dbscan.sh" "BASIC_DBSCAN/basic_dbscan.sh" "BASIC_DBSCAN/basic_dbscan_opt.sh" "BASIC_DBSCAN++/basic_dbscan++.sh" "KDTREE_DBSCAN/kdtree_dbscan.sh" "KDTREE_DBSCAN/kdtree_dbscan_opt.sh" "MLPACK_DBSCAN/mlpack_dbscan.sh")

# generate random points : NUM_CLUSTERS * POINTS_PER_CLUSTER = number of points generated 
# NUM_CLUSTERS=7
# POINTS_PER_CLUSTER=97

# Generate random points
# echo "Generating the random points input file..."
# python3 points_generator.py --num_clusters $NUM_CLUSTERS --points_per_cluster $POINTS_PER_CLUSTER
# echo "Random points generated."

input_file=$1   # INPUTS/CSVs/obstacles239.csv has the most points of all (the files from the same folder)
eps=$2          # minimize --> current value on nduja 0.35 --> Recommended eps = 
min_pts=$3      # maximize --> current value on nduja 2 --> Recommended minPts = 2*DIM = 2*3 = 6
norm_type=$4    # Recommended = 2

# Empty the compare.txt file or create it if it doesn't exist
> compare.txt

# echo "Generating results using $NUM_CLUSTERS clusters and $POINTS_PER_CLUSTER points per cluster" >> compare.txt
echo "input = $input_file, eps = $eps, min_pts = $min_pts, norm_type = $norm_type" >> compare.txt
echo -e "\n\n" >> compare.txt

echo "Ensuring all scripts are executable..."
# Ensure scripts are executable
for script in "${scripts[@]}"; do
    chmod +x "$script" || echo "Failed to set executable flag on $script"
done

echo "Executing scripts..."
# Execute each script in order and redirect output to compare.txt
for script in "${scripts[@]}"; do
    echo "--------------------------------------------------------------------------------------------------------------------------" >> compare.txt
    echo "Running $script..." >> compare.txt
    ./"$script" "../$input_file" $eps $min_pts $norm_type 2>&1 >> compare.txt
    csv="${script/.sh/.csv}"
    echo -ne "\trand_index:\t\t" >> compare.txt
    python3 ./cluster_compare.py --input $input_file --output $csv --eps $eps --min_pts $min_pts >> compare.txt
    echo "--------------------------------------------------------------------------------------------------------------------------" >> compare.txt
    echo -e "\n\n" >> compare.txt
    if [ $? -ne 0 ]; then
        echo "$script failed"
        exit 1
    fi
    echo "$script completed successfully."
done

echo "All scripts executed successfully."
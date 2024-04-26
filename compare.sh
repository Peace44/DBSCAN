#!/bin/bash

# Define an array with script names
scripts=("BASIC_DBSCAN/basic_dbscan.sh"  "KDTREE/KDTREE.sh" "MLPACK_DBSCAN/mlpack_dbscan.sh")

# generate random points : NUM_CLUSTERS * POINTS_PER_CLUSTER = number of points generated 
NUM_CLUSTERS=50
POINTS_PER_CLUSTER=2000

# Generate random points
echo "Generating the random points input file..."
python3 points_generator.py --num_clusters $NUM_CLUSTERS --points_per_cluster $POINTS_PER_CLUSTER
echo "Random points generated."

# Define input file
input_file="../INPUTS/random_points.csv"


# Set default values of DBSCAN parameters: eps and minPts
eps=2.0
minPts=2

# Empty the compare.txt file or create it if it doesn't exist
> compare.txt

echo "Ensuring all scripts are executable..."
echo "Generating results using $NUM_CLUSTERS clusters and $POINTS_PER_CLUSTER points per cluster" >> compare.txt
echo "eps = $eps, minPts = $minPts" >> compare.txt
echo "" >> compare.txt

# Ensure scripts are executable
for script in "${scripts[@]}"; do
    chmod +x "$script" || echo "Failed to set executable flag on $script"
done

echo "Executing scripts..."
# Execute each script in order and redirect output to compare.txt
for script in "${scripts[@]}"; do
    echo "Running $script..." >> compare.txt
    ./"$script" $input_file $eps $minPts >> compare.txt 2>&1
    python3 ./cluster_compare.py >> compare.txt
    echo "" >> compare.txt
    if [ $? -ne 0 ]; then
        echo "$script failed"
        exit 1
    fi
    echo "$script completed successfully."
done

echo "All scripts executed successfully."

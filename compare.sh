#!/bin/bash

# Define an array with script names
scripts=("BASIC_DBSCAN/basic_dbscan.sh"  "KDTREE/KDTREE.sh" "MLPACK_DBSCAN/mlpack_dbscan.sh")
# Empty the compare.txt file or create it if it doesn't exist
> compare.txt

echo "Ensuring all scripts are executable..."
# Ensure scripts are executable
for script in "${scripts[@]}"; do
    chmod +x "$script" || echo "Failed to set executable flag on $script"
done

echo "Executing scripts..."
# Execute each script in order and redirect output to compare.txt
for script in "${scripts[@]}"; do
    echo "Running $script..." >> compare.txt
    ./"$script" >> compare.txt 2>&1
    python3 ./cluster_compare.py >> compare.txt
    echo "" >> compare.txt
    if [ $? -ne 0 ]; then
        echo "$script failed"
        exit 1
    fi
    echo "$script completed successfully."
done

echo "All scripts executed successfully."

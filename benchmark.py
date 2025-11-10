import os
import subprocess
import csv
import argparse
import time
import numpy as np

# Argument parser configuration
parser = argparse.ArgumentParser(description='Benchmark DBSCAN program on multiple CSV files.')
parser.add_argument('--dbscan_program', type=str, help='Path to the DBSCAN program executable')
parser.add_argument('--input_dir', type=str, help='Directory containing CSV files')
parser.add_argument('--eps', type=float, help='Epsilon parameter for DBSCAN')
parser.add_argument('--min_pts', type=int, help='Minimum points parameter for DBSCAN')
parser.add_argument('--norm_type', type=str, help='Norm type for DBSCAN (1, 2, inf)')
args = parser.parse_args()

# Configuration from command line arguments
dbscan_program = args.dbscan_program
input_dir = args.input_dir
eps = args.eps
min_pts = args.min_pts
norm_type = args.norm_type

if norm_type == "1":
    metric = "manhattan"
elif norm_type == "2":
    metric = "euclidean"
else:
    metric = "chebyshev"


# Extract the name of the DBSCAN program executable
exe_name = os.path.basename(dbscan_program)
output_file = f"BENCHMARKS/{exe_name}_{eps}_{min_pts}_{norm_type}_benchmark.csv"

# Collect all CSV files from the input directory
csv_files = [f for f in os.listdir(input_dir) if f.endswith('.csv')]

# Open the output CSV file for writing
with open(output_file, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["filename", "duration_microseconds", "rand_index"])  # Write the header

    num_files = 0

    durations = []
    rand_indexes = []

    # Iterate over each CSV file
    for csv_file in csv_files:
        input_file = os.path.join(input_dir, csv_file)
        
        # Run the DBSCAN program and capture the output
        result = subprocess.run(
            [dbscan_program, input_file, str(eps), str(min_pts), norm_type],
            capture_output=True,
            text=True
        )
        
        # Extract the duration from the output
        duration = int(result.stdout.strip())

        time.sleep(0.001)

        # Locate the dbscan_program_output
        dbscan_program_output = dbscan_program + ".csv"

        # Run cluster_compare.py and capture the Rand Index
        compare_result = compare_result = subprocess.run(
            ['python3', 'cluster_compare.py', '--input', input_file, '--output', dbscan_program_output, '--eps', str(eps), '--min_pts', str(min_pts), '--metric', metric],
            capture_output=True,
            text=True
        )

        rand_index = float(compare_result.stdout.strip())

        # Write the result to the CSV file
        writer.writerow([csv_file, duration, rand_index])
        
        # Update file count
        num_files += 1

        # Update durations and rand_indexes
        durations.append(duration)
        rand_indexes.append(rand_index)

# Calculate average duration and average rand_index
median_duration = np.median(durations)
median_rand_index = np.median(rand_indexes)

average_duration = np.mean(durations)
average_rand_index = np.mean(rand_indexes)

print(f"Benchmarking complete. Results are saved in {output_file}")

print(f"Median execution time: {median_duration} microseconds")
print(f"Median Rand Index: {median_rand_index}")

print(f"Average execution time: {average_duration} microseconds")
print(f"Average Rand Index: {average_rand_index}")
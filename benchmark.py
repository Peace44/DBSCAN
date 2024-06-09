import os
import subprocess
import csv
import argparse
import time

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

# Extract the name of the DBSCAN program executable
exe_name = os.path.basename(dbscan_program)
output_file = f"BENCHMARKS/{exe_name}_{eps}_{min_pts}_{norm_type}_benchmark.csv"

# Collect all CSV files from the input directory
csv_files = [f for f in os.listdir(input_dir) if f.endswith('.csv')]

# Open the output CSV file for writing
with open(output_file, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["filename", "duration_microseconds", "rand_index"])  # Write the header

    total_duration = 0
    total_rand_index = 0
    num_files = 0

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
            ['python3', 'cluster_compare.py', '--input', input_file, '--output', dbscan_program_output, '--eps', str(eps), '--min_pts', str(min_pts)],
            capture_output=True,
            text=True
        )

        rand_index = float(compare_result.stdout.strip())

        # Write the result to the CSV file
        writer.writerow([csv_file, duration, rand_index])
        
        # Update total duration, total rand_index, and file count
        total_duration += duration
        total_rand_index += rand_index
        num_files += 1

# Calculate average duration and average rand_index
average_duration = total_duration / num_files if num_files > 0 else 0
average_rand_index = total_rand_index / num_files if num_files > 0 else 0

print(f"Benchmarking complete. Results are saved in {output_file}")
print(f"Average execution time: {average_duration} microseconds")
print(f"Average Rand Index: {average_rand_index}")

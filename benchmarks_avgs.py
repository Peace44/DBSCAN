import os
import csv

# Define the directory containing the benchmark CSV files
directory = "BENCHMARKS"
out_directory = "BENCHMARKS_AVGS"

# Ensure the directory exists
if not os.path.exists(directory):
    print(f"Directory '{directory}' not found.")
    exit()

os.makedirs(out_directory, exist_ok=True)

# Process each CSV file in the directory
for filename in os.listdir(directory):
    if filename.endswith(".csv") and not filename.endswith("_avg.csv"):
        filepath = os.path.join(directory, filename)

        times = []
        accuracies = []

        with open(filepath, newline='') as csvfile:
            reader = csv.reader(csvfile)
            header = next(reader)  # Skip header

            for row in reader:
                try:
                    time = float(row[1])
                    accuracy = float(row[2])
                    times.append(time)
                    accuracies.append(accuracy)
                except (IndexError, ValueError):
                    print(f"Skipping malformed row in {filename}: {row}")

        if times and accuracies:
            avg_time = sum(times) / len(times)
            avg_accuracy = sum(accuracies) / len(accuracies)

            output_filename = filename + "_avg.csv"
            output_path = os.path.join(out_directory, output_filename)

            with open(output_path, mode='w', newline='') as outfile:
                writer = csv.writer(outfile)
                writer.writerow(["filename", "average_time", "average_accuracy"])
                writer.writerow([filename, f"{avg_time:.4f}", f"{avg_accuracy:.4f}"])

            print(f"Averages written to {output_filename}")
        else:
            print(f"No valid data in {filename}")

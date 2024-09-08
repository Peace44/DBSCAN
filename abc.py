import os
import re
import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import time

# Directory where your CSV files are stored
csv_directory = './INPUTS/csvs'

# Function to extract the number from the filename (e.g., 'cloud123.csv' -> 123)
def extract_number(filename):
    match = re.search(r'\d+', filename)
    return int(match.group()) if match else float('inf')  # Handle cases without numbers

# List all CSV files in the directory
csv_files = sorted([f for f in os.listdir(csv_directory) if f.endswith('.csv')], key=extract_number)

# Create a figure and 3D axis once (reuse for all plots)
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Function to visualize each CSV file as a 3D scatter plot
def visualize_csv_3d(csv_file):
    # Read the CSV
    data = pd.read_csv(os.path.join(csv_directory, csv_file))
    
    # Check if the file has at least 3 columns to plot
    if data.shape[1] < 3:
        print(f"Skipping {csv_file}, not enough columns.")
        return
    
    # Assuming the first three columns represent the 3D points to plot
    x = data.iloc[:, 0]
    y = data.iloc[:, 1]
    z = data.iloc[:, 2]

    # Clear previous plot
    ax.cla()

    # Create 3D scatter plot
    ax.scatter(x, y, z, c='blue', marker='o', s=1)

    # Set labels
    ax.set_xlabel('X Label')
    ax.set_ylabel('Y Label')
    ax.set_zlabel('Z Label')
    
    # Set the title as the filename
    ax.set_title(csv_file)

    # Draw the updated plot
    plt.draw()

    # Pause to update the plot (non-blocking)
    plt.pause(0.01)  # Adjust the pause duration as needed

# Show the figure window
plt.ion()  # Turn on interactive mode to keep the window open

# Loop through all CSV files and generate visualizations
for csv_file in csv_files:
    visualize_csv_3d(csv_file)
    
    # Wait 1 millisecond between visualizations
    #time.sleep(0.001)

# Turn off interactive mode when done
plt.ioff()

# Keep the window open after the last plot
plt.show()


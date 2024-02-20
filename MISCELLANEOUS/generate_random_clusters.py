from sklearn.datasets import make_blobs
import pandas as pd
import numpy as np
import os

def generate_and_save_datasets(n_samples_list, n_features, centers_range, cluster_std_range, file_prefix):
    for n_samples in n_samples_list:
        # Randomly choose the number of centers
        n_centers = np.random.randint(centers_range[0], centers_range[1] + 1)
        
        # Generate random cluster standard deviations
        cluster_stds = np.random.uniform(cluster_std_range[0], cluster_std_range[1], n_centers)
        
        # Generate the dataset
        X, _ = make_blobs(n_samples=n_samples, n_features=n_features, centers=n_centers, cluster_std=cluster_stds)
        
        # Convert to DataFrame
        df = pd.DataFrame(X, columns=['x', 'y', 'z'])
        
        # Construct file name
        file_name = f"{file_prefix}{n_samples}randPts.csv"
        
        # Save to CSV
        df.to_csv(file_name, index=False)
        print(f"Dataset with {n_samples} points and {n_centers} varied centers saved to {file_name}")

# Parameters
n_samples_list = [10**3, 10**4, 10**5, 10**6, 10**7]
n_features = 3
centers_range = (2**4, 2**5)  # Randomly choose between 16 and 32 centers
cluster_std_range = (3**-1, 3**+1)  # Random cluster standard deviations between 0.33 and 3.0
file_prefix = os.path.dirname(__file__) + '/../INPUTS/'

# Generate and save the datasets
generate_and_save_datasets(n_samples_list, n_features, centers_range, cluster_std_range, file_prefix)

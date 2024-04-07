import pandas as pd
from sklearn.cluster import DBSCAN
from sklearn.metrics import rand_score
from sklearn.preprocessing import StandardScaler

# Step 1: Load the dataset for DBSCAN
df = pd.read_csv('INPUTS/random_points.csv', header=None)
#skip first row
df = df.iloc[1:]
points = df.values[:, :3]  # Assuming the points are in the first three columns

# It's often a good idea to scale the data for clustering algorithms
#scaler = StandardScaler()
#points_scaled = scaler.fit_transform(points)

# Step 2: Perform DBSCAN clustering
dbscan = DBSCAN(eps=2.0, min_samples=2) 
clusters = dbscan.fit_predict(points)

# Step 3: Load the dataset with existing cluster assignments
df_clusters = pd.read_csv('OUTPUTS/random_points_clusters.csv', header=None)
#skip first row
df_clusters = df_clusters.iloc[1:]
true_clusters = df_clusters.values[:, 3]  # Assuming the cluster labels are in the fourth column

# Step 4: Compute the Rand Index
rand_index = rand_score(true_clusters, clusters)
print(f'Rand Index: {rand_index}')

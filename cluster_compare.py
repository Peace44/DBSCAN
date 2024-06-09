import pandas as pd
from sklearn.cluster import DBSCAN
from sklearn.metrics import rand_score
from sklearn.preprocessing import StandardScaler
import argparse 

EPS = 0.35      # As found on the NDUJA
MIN_PTS = 2     # As found on the NDUJA

parser = argparse.ArgumentParser()
parser.add_argument('--input', type=str, default="")
parser.add_argument('--output', type=str, default="")
parser.add_argument('--eps', type=float, default=EPS)
parser.add_argument('--min_pts', type=int, default=MIN_PTS)
args = parser.parse_args()
input = args.input
output = args.output
eps  = args.eps
min_pts = args.min_pts



# Step 1: Load the dataset for DBSCAN
df = pd.read_csv(input, header=None)
#skip first row
df = df.iloc[1:]
points = df.values[:, :3]  # Assuming the points are in the first three columns


# Step 2: Perform DBSCAN clustering
dbscan = DBSCAN(eps=eps, min_samples=min_pts) 
clusters = dbscan.fit_predict(points)

# Step 3: Load the dataset with existing cluster assignments
df_clusters = pd.read_csv(output, header=None)
#skip first row
df_clusters = df_clusters.iloc[1:]
true_clusters = df_clusters.values[:, 3]  # Assuming the cluster labels are in the fourth column

# Step 4: Compute the Rand Index
rand_index = rand_score(true_clusters, clusters)
print(f'{rand_index}')

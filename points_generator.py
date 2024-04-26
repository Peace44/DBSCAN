import numpy as np
import pandas as pd
import argparse 

np.random.seed(42)


index_points = []


NUM_CLUSTERS = 50
points_per_cluster = 2000

parser = argparse.ArgumentParser()
parser.add_argument('--num_clusters', type=int, default=NUM_CLUSTERS)
parser.add_argument('--points_per_cluster', type=int, default=points_per_cluster)
args = parser.parse_args()
NUM_CLUSTERS = args.num_clusters
points_per_cluster = args.points_per_cluster


list_of_points = []
for c in range(NUM_CLUSTERS):
    x_cluster = np.random.uniform(-100,100)
    y_cluster = np.random.uniform(-100,100)

    for i in range(points_per_cluster):
        list_of_points.append(np.random.multivariate_normal([x_cluster,y_cluster,0], [[1,0,0],[0,1,0],[0,0,3]]))
        index_points.append(c)
points = np.array(list_of_points)
points_x = points[:,0]
points_y = points[:,1]
points_z = points[:,2]



df = pd.DataFrame()
df["x"] = points_x
df["y"] = points_y
df["z"] = points_z
#df["cluster"] = np.array(index_points)


df_index = list(range(len(df)))
np.random.shuffle(df_index)
df = df.iloc[df_index]
df 
df.to_csv("./INPUTS/random_points.csv", index=False)
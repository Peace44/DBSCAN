import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns  # easier heatmap than raw matplotlib

df = pd.read_csv("dbscan_benchmarks_all_versions.csv")


# plt.figure(figsize=(8,6))
# scatter = plt.scatter(
#     subdf["average_execution_time"],
#     subdf["average_rand_index"],
#     c=subdf["eps"],       # color by eps
#     s=subdf["minPts"]*40, # size by minPts
#     cmap="viridis",
#     alpha=0.7
# )

# plt.colorbar(scatter, label="eps")
# plt.xlabel("Average Execution Time")
# plt.ylabel("Average Rand Index")
# plt.title(f"Performance trade-off for {algo}")
# plt.grid(True)

# for _, row in subdf.iterrows():
#     plt.text(
#         row["average_execution_time"], 
#         row["average_rand_index"], 
#         f'n={row["norm_type"]}', fontsize=8
#     )

# plt.show()


# pivot = subdf.pivot_table(
#     index="eps", columns=["minPts","norm_type"], values="average_execution_time"
# )

# plt.figure(figsize=(10,6))
# sns.heatmap(pivot, annot=True, fmt=".1f", cmap="YlGnBu")
# plt.title(f"Execution Time for {algo}")
# plt.ylabel("eps")
# plt.xlabel("(minPts, norm_type)")
# plt.show()



# pivot = subdf.pivot_table(
#     index="eps", columns=["minPts","norm_type"], values="average_rand_index"
# )

# plt.figure(figsize=(10,6))
# sns.heatmap(pivot, annot=True, fmt=".1f", cmap="YlGnBu")
# plt.title(f"Rand Index for {algo}")
# plt.ylabel("eps")
# plt.xlabel("(minPts, norm_type)")
# plt.show()

from sklearn.preprocessing import MinMaxScaler

algos = df["algorithm"].unique()

for algo in algos:

    subdf = df[df["algorithm"] == algo] 
    metrics = subdf[["average_execution_time","average_rand_index", "median_execution_time" , "median_rand_index"]].copy()
    scaler = MinMaxScaler()
    metrics_scaled = scaler.fit_transform(metrics)

    # Execution time is better when smaller → invert
    score = (1 - metrics_scaled[:,0]) + metrics_scaled[:,1]

    subdf = subdf.assign(score=score)

    best = subdf.loc[subdf["score"].idxmax()]
    print("Best configuration:", best)
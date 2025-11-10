import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Assume df is built from your benchmark results
# Columns: algorithm, eps, minPts, norm_type, median_execution_time, average_execution_time, median_rand_index, average_rand_index
df = pd.read_csv("dbscan_benchmarks_all_versions.csv")


# --- A: Comparison by algorithm ---
eps=0.25
minPts=2
norm_type=1


subset = df[
    (df['eps'] == eps) &
    (df['minPts'] == minPts) &
    (df['norm_type'] == norm_type)
    # & (df['algorithm'].isin(["old_dbscan", "kdtree_dbscan", "basic"]))
]
subset.plot(x="algorithm", y=["median_execution_time", "average_execution_time"], kind="bar")
plt.title(f"Execution time comparison (eps={eps}, minPts={minPts}, norm_type={norm_type})", fontsize=18)
plt.ylabel("Execution Time (μs)", fontsize=18)
plt.xticks(rotation=0,fontsize=14 )  # Set x-axis labels horizontal
plt.yticks(fontsize=14)
# plt.ylim(0.90, 1.0)
plt.legend(fontsize=14)
plt.xlabel("Algorithm", fontsize=16)
plt.show()

subset.plot(x="algorithm", y=["median_rand_index", "average_rand_index"], kind="bar")
plt.title(f"Rand Index comparison (eps={eps}, minPts={minPts}, norm_type={norm_type})", fontsize=18)
plt.xlabel("Algorithm", fontsize=16)
plt.ylabel("Rand Index", fontsize=16)
plt.xticks(rotation=0, fontsize=14)
plt.yticks(fontsize=14)
plt.ylim(0.90, 1.0)
plt.legend(fontsize=14)
plt.show()


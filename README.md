# DBSCAN comparison script

## Prerequisites
### MLPACK
Make sure the mlpack library is installed. It can be installed through `sudo apt install libmlpack-dev mlpack-bin libarmadillo-dev` (linux). checkout [mlpack](https://www.mlpack.org/getstarted.html) for more information.

### python libraries
In order to generate the random points for the input file, you should have the following libraries isntalled:
- numpy
- pandas
- sklearn

Can use `pip install numpy pandas sklearn` to install them.




## Running the script
Just run "./compare.sh", it will run: "basic_dbscan.sh", "kdtree.sh" and "mlpack_dbscan.sh". The scripts must be in their respective folders, otherwise change in "./compare.sh" the "scripts" array, which contains the paths to all the scripts that have to be executed.The output of the ocmparison is stored in compare.txt.

## Changing comparison variables
### DBSCAN parameters
Inside the "compare.sh" script the following variables can be changed in order to make different comparisons and clusterings:

- "eps" sets the epsilon parameter of the dbscan algorithm.
- "minPts" sets the minimum points that should belong to a cluster.

### Input file
The input file generation can be changed in the "compare.sh" script.
Change the variable NUM_CLUSTERS and POINTS_PER_CLUSTER to generate different number of clusters and points per cluster. The number of points generated in the input file will be NUM_CLUSTERS * POINTS_PER_CLUSTER.
# DBSCAN comparison script

## Prerequisites
Make sure the mlpack library is installed. It can be installed through `sudo apt install libmlpack-dev mlpack-bin libarmadillo-dev` (linux). checkout [mlpack](https://www.mlpack.org/getstarted.html) for more information.

## Running the script
Just run "./compare.sh", it will run: "basic_dbscan.sh", "kdtree.sh" and "mlpack_dbscan.sh". The scripts must be in their respective folders, otherwise change in "./compare.sh" the "scripts" array, which contains the paths to all the scripts that have to be executed.The output of the ocmparison is stored in compare.txt.

## Changing comparison variables

Inside the "compare.sh" script the following variables can be changed in order to make different comparisons and clusterings:

- "eps" sets the epsilon parameter of the dbscan algorithm.
- "minPts" sets the minimum points that should belong to a cluster.
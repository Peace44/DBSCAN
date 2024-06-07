#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <map>
#include <pcl/impl/point_types.hpp>

const int NOISE = -2;
const int UNCLASSIFIED = -1;

class DBSCAN {
public:
    DBSCAN(double eps, int minPts, double centroideMaxDistance, double sensorHeight, double maxDistToLine, double xFilter, std::vector<pcl::PointXYZ> points);
    void run ();
    
    void dfs (int now, int c);
    void checkNearPoints();
    bool isCoreObject(int idx);
    
    std::vector<std::vector<int>> getCluster();

    double getDis(pcl::PointXYZ a, pcl::PointXYZ b);

    void computeCentroids();
    void filterCentroids();
    std::vector<pcl::PointXYZ> getCentroids();

private:
    int minPts;
    double eps;
    double centroideMaxDistance;
    double sensorHeight;
    double maxDistToLine;
    double xFilter;

    std::vector<pcl::PointXYZ> points;
    int size;
    std::vector<int> ptsCnt;
    std::vector<int> pointsToCluster;
    std::vector<std::vector<int> > adjPoints;
    std::vector<std::vector<int>> clusterToPoints; // per ogni cluster, vettore con gli indici dei punti
    int clusterIdx;
    std::vector<pcl::PointXYZ> centroids;
};
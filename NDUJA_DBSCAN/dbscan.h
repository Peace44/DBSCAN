#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <map>



const int NOISE = -2;
const int UNCLASSIFIED = -1;

struct Point3D {
    double x, y, z;
    int cluster = UNCLASSIFIED;
};

using distance_function = double(*)(const Point3D&, const Point3D&);



class DBSCAN {
public:
    DBSCAN(std::vector<Point3D> points, double eps, int minPts, distance_function dist_func, double centroideMaxDistance, double sensorHeight, double maxDistToLine, double xFilter);
    void run ();
    
    void dfs (int now, int c);
    void checkNearPoints();
    bool isCoreObject(int idx);
    
    std::vector<std::vector<int>> getCluster();

    double getDis(const Point3D& a, const Point3D& b);

    void computeCentroids();
    void filterCentroids();
    std::vector<Point3D> getCentroids();

private:
    std::vector<Point3D> points;

    int minPts;
    double eps;

    distance_function dist_func;
    
    double centroideMaxDistance;
    double sensorHeight;
    double maxDistToLine;
    double xFilter;

    int size; // size of points
    std::vector<int> ptsCnt;
    std::vector<int> pointsToCluster;
    std::vector<std::vector<int> > adjPoints;
    std::vector<std::vector<int>> clusterToPoints; // per ogni cluster, vettore con gli indici dei punti
    int clusterIdx;
    std::vector<Point3D> centroids;
};
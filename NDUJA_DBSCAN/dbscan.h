#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <map>
#include "common/point.h"

const int NOISE = -2;
const int NOT_CLASSIFIED = -1;

class DBSCAN {
public:
    DBSCAN(double eps, int minPts, double centroideMaxDistance, double sensorHeight, double maxDistToLine, double xFilter, std::vector<Point> points);
    void run ();
    
    void dfs (int now, int c);
    void checkNearPoints();
    bool isCoreObject(int idx);
    
    std::vector<std::vector<int>> getCluster();

    double getDis(Point a, Point b);

    void computeCentroids();
    void filterCentroids();
    std::vector<Point> getCentroids();

private:
    int minPts;
    double eps;
    double centroideMaxDistance;
    double sensorHeight;
    double maxDistToLine;
    double xFilter;

    std::vector<Point> points;
    int size;
    std::vector<int> ptsCnt;
    std::vector<int> pointsToCluster;
    std::vector<std::vector<int> > adjPoints;
    std::vector<std::vector<int>> clusterToPoints; // per ogni cluster, vettore con gli indici dei punti
    int clusterIdx;
    std::vector<Point> centroids;
};
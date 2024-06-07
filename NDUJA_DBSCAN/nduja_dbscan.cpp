#include "dbscan.h"



DBSCAN::DBSCAN(std::vector<Point3D> points, double eps, int minPts, distance_function dist_func)
{
    this->points = points;
    this->eps = eps;
    this->minPts = minPts;
    this->dist_func = dist_func;

    this->size = (int)points.size();
    adjPoints.resize(size);
    this->clusterIdx=UNCLASSIFIED;
    this->ptsCnt = std::vector<int>(this->size, 0);
    this->pointsToCluster = std::vector<int>(this->size, UNCLASSIFIED);
}



void DBSCAN::run () 
{
    // analyze near points
    checkNearPoints();

    // assign clusters
    for(int i=0;i<size;i++) {
        if(pointsToCluster[i] != UNCLASSIFIED) continue;
        
        if(isCoreObject(i)) dfs(i, ++clusterIdx);
        else pointsToCluster[i] = NOISE;
    }
    
    // aggregate by cluster, filter noise
    clusterToPoints.resize(clusterIdx+1);
    for(int i=0;i<size;i++) {
        if(pointsToCluster[i] != NOISE) {
            clusterToPoints[pointsToCluster[i]].push_back(i);
        }
    }
}



void DBSCAN::dfs (int now, int c) 
{
    pointsToCluster[now] = c;
    if(!isCoreObject(now)) return;
    
    for(auto&next:adjPoints[now]) {
        // if(points[next].cluster != UNCLASSIFIED) continue;
        if(pointsToCluster[next] != UNCLASSIFIED) continue;
        dfs(next, c);
    }
}



void DBSCAN::checkNearPoints() 
{
    // TODO: j=i+1
    for(int i=0;i<size;i++) {
        for(int j=0;j<size;j++) {
            if (i == j) continue;
            if (getDis(points[i], points[j]) <= eps) {
                ptsCnt[i]++;
                adjPoints[i].push_back(j);
            }
        }
    }
}



bool DBSCAN::isCoreObject(int idx) 
{
    return ptsCnt[idx] >= minPts;
}



std::vector<std::vector<int>> DBSCAN::getCluster() 
{
    return clusterToPoints;
}



double DBSCAN::getDis(const Point3D& a, const Point3D& b)
{  
    return dist_func(a, b);
}
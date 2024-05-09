#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <map>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/common/distances.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include "dbscan/dbscan.h"






// Filter by cone shape params
#define THRESHOLD 0.06

#define CONE_HEIGHT 0.36
#define CENTR_DIAM_RATIO 0.18
#define BASE_DIAM 0.25

#define TAN_ALPHA (BASE_DIAM / (2*CONE_HEIGHT))









DBSCAN::DBSCAN(double eps, int minPts, double centroideMaxDistance, double sensorHeight, double maxDistToLine, double xFilter, std::vector<pcl::PointXYZ> points) {
    this->eps = eps;
    this->minPts = minPts;
    this->centroideMaxDistance = centroideMaxDistance;
    this->sensorHeight = sensorHeight;
    this->maxDistToLine = maxDistToLine;
    this->xFilter = xFilter;

    this->points = points;
    this->size = (int)points.size();
    adjPoints.resize(size);
    this->clusterIdx=-1;

    this->ptsCnt = std::vector<int>(this->size, 0);
    this->pointsToCluster = std::vector<int>(this->size, NOT_CLASSIFIED);
}

void DBSCAN::run () {
    // analyze near points
    checkNearPoints();
    // assign clusters
    for(int i=0;i<size;i++) {
        // if(clusterToPoints[i] != NOT_CLASSIFIED) continue;
        if(pointsToCluster[i] != NOT_CLASSIFIED) continue;
        
        if(isCoreObject(i)) {
            dfs(i, ++clusterIdx);
        } else {
            pointsToCluster[i] = NOISE;
        }
    }
    
    // aggregate by cluster, filter noise
    clusterToPoints.resize(clusterIdx+1);
    for(int i=0;i<size;i++) {
        if(pointsToCluster[i] != NOISE) {
            clusterToPoints[pointsToCluster[i]].push_back(i);
        }
    }

    //compute centroids 
    computeCentroids();

    filterCentroids();

}

void DBSCAN::dfs (int now, int c) {
    pointsToCluster[now] = c;
    if(!isCoreObject(now)) return;
    
    for(auto&next:adjPoints[now]) {
        // if(points[next].cluster != NOT_CLASSIFIED) continue;
        if(pointsToCluster[next] != NOT_CLASSIFIED) continue;
        dfs(next, c);
    }
}

void DBSCAN::checkNearPoints() {
    // TODO: j=i+1
    for(int i=0;i<size;i++) {
        for(int j=0;j<size;j++) {
            if(i==j) continue;
            if(getDis(points[i],points[j]) <= eps) {
                ptsCnt[i]++;
                adjPoints[i].push_back(j);
            }
        }
    }
}

bool DBSCAN::isCoreObject(int idx) {
    return ptsCnt[idx] >= minPts;
}

std::vector<std::vector<int>> DBSCAN::getCluster() {
    return clusterToPoints;
}

double DBSCAN::getDis(pcl::PointXYZ a, pcl::PointXYZ b) {
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

void DBSCAN::computeCentroids(){
    int count;
    centroids.resize(clusterIdx+1);
    
    for(int i=0; i<clusterIdx+1; i++){
        centroids[i].x = 0;
        centroids[i].y = 0;
        centroids[i].z = 0;
        count = (int)clusterToPoints[i].size();

        for(int j=0; j<count; j++){
            centroids[i].x += points[ clusterToPoints[i][j] ].x;
            centroids[i].y += points[ clusterToPoints[i][j] ].y;
            centroids[i].z += points[ clusterToPoints[i][j] ].z;
        }

        centroids[i].x = centroids[i].x / count;
        centroids[i].y = centroids[i].y / count;
        centroids[i].z = centroids[i].z / count;
    }
}

void DBSCAN::filterCentroids(){
    std::vector<int> to_delete;

    for(int cluster_index = 0; cluster_index < centroids.size(); cluster_index++)
    {
        std::vector<int> cluster_points = clusterToPoints[cluster_index];
        pcl::PointXYZ cluster_centroid = centroids[cluster_index];

        bool stop = false;
        
        // Filter by height
        if(cluster_centroid.z < -sensorHeight + maxDistToLine)
        {
            to_delete.insert(to_delete.begin(), cluster_index);
            stop = true;
        }

        if(stop)
            continue;

        

        // Filter by x distance
        if(cluster_centroid.x < xFilter)
        {
            to_delete.insert(to_delete.begin(), cluster_index);
            stop = true;
        }

        if(stop)
            continue;
    
        // Filter by centroid distance
        
        for(int point_index : cluster_points)
        {   
            if(pcl::euclideanDistance(cluster_centroid, points[point_index]) > centroideMaxDistance)
            {
                to_delete.insert(to_delete.begin(), cluster_index);
                stop = true;
                break;
            }
        }

        if(stop)
            continue;


        // Filter by max z delta between points
        double max_delta_z = 0;

        for(int point_i1 : cluster_points)
        {   
            for(int point_i2 : cluster_points)
            {
                double delta_z = points[point_i1].z - points[point_i2].z;
                if(delta_z > max_delta_z)
                    max_delta_z = delta_z;
            }
        }

        if(max_delta_z < 0.05)
        {
            to_delete.insert(to_delete.begin(), cluster_index);
            continue;
        }




        // Filter by cone shape
        double score = 0;
        Eigen::Vector3d A = Eigen::Vector3d(cluster_centroid.x, cluster_centroid.y, CONE_HEIGHT - sensorHeight);
        Eigen::Vector2d deltaA = (A.head<2>() / A.head<2>().norm()) * CENTR_DIAM_RATIO * BASE_DIAM;
        A[0] += deltaA[0];
        A[1] += deltaA[1];

        Eigen::Vector3d b = Eigen::Vector3d(0, 0, -CONE_HEIGHT);

        for(int point_index : cluster_points)
        {
            Eigen::Vector3d P = Eigen::Vector3d(points[point_index].x, points[point_index].y, points[point_index].z);
            Eigen::Vector3d AP = P - A;
            Eigen::Vector3d d = AP - ((AP.dot(b)) / (CONE_HEIGHT*CONE_HEIGHT)) * b;
            Eigen::Vector3d C = A + b + (d/d.norm()) * TAN_ALPHA * CONE_HEIGHT;
            
            double v = 0;

            if((AP.dot(C-A)) < 0)
                v = AP.norm();
            else if((P-C).dot(A-C) < 0)
                v = (P-C).norm();
            else
                v = (AP - (AP.dot(C-A))/((C-A).dot(C-A)) * (C-A)).norm();
            
            double v_threshold_ratio = v*v/(THRESHOLD*THRESHOLD);

            if(v_threshold_ratio<1)
                score += (1 - v_threshold_ratio);
        
        }

        score /= cluster_points.size();
        // std::cout << "score: " << score << std::endl << std::endl;

        if(score < 0.55)
        {
            to_delete.insert(to_delete.begin(), cluster_index);
            continue;
        }
    }

    for(int centroid_index : to_delete)
        centroids.erase(std::next(centroids.begin(), centroid_index));
}


std::vector<pcl::PointXYZ> DBSCAN::getCentroids(){
    return centroids;
}

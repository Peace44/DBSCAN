#include "KDTree.h"
#include <iostream>
#include <vector>

#define UNCLASSIFIED -1

struct Point3D {
    double x, y, z;
    int cluster = UNCLASSIFIED;
   
};


int main() {
    kdtree* tree = kd_create(3); // Build the KD-tree
    double eps = 1.0;
    double epsSquared = eps * eps; // Precompute eps squared
    int point_id = 0;

    //create vector of points
    std::vector<Point3D> points = {
        {0, 0, 0},
        {0, 0, 1},
        {0, 1, 0},
        {0, 1, 1},
        {1, 0, 0},
        {1, 0, 1},
        {1, 1, 0},
        {1, 1, 1}
    };   
    //populate kdtree
     for (size_t i = 0; i < points.size(); i++) {
        double pos[3] = {points[i].x, points[i].y, points[i].z};
        kd_insert(tree, pos, (void*)&points[i]);
    }

    std::vector<void*> seeds;
 
    std::vector<double> query = {points[point_id].x, points[point_id].y, points[point_id].z};
    double pos[3] = {query[0], query[1], query[2]};
    kdres *result;

    //print query
    std::cout << "Query: " << query[0] << ", " << query[1] << ", " << query[2] << std::endl;
    std::cout << "Pos: " << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;
    result = kd_nearest_range(tree, pos, eps);;
    
    //print result
    std::cout << "Result: " << std::endl;
    if (result->size == 0) {
        std::cout << "No result" << std::endl;
    }
    std::vector<void*> sseds;
    while (kd_res_end(result)==0) {
        void *p = kd_res_item(result, NULL);
        seeds.push_back(p);
        Point3D *point = static_cast<Point3D*>(p);
        std::cout << "Point: " << point->x << ", " << point->y << ", " << point->z << std::endl;
        
        kd_res_next(result);
    }
    kd_res_free(result);
    kd_free(tree);
    std::cout << "Seeds: " << std::endl;
    for (auto seed : seeds) {
        static_cast<Point3D*>(seed)->cluster = 1;
        std::cout << "Seed: " << static_cast<Point3D*>(seed)->x << ", " << static_cast<Point3D*>(seed)->y << ", " << static_cast<Point3D*>(seed)->z << static_cast<Point3D*>(seed)->cluster << std::endl;
    }
    return 0;

}
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <cstring>
#include <charconv>
#include <chrono>

 #define KDTREE true

 #if KDTREE
 #include "../KDTREE/KDTree.h"
 #endif

const int NOISE = -1;
const int UNCLASSIFIED = 0;


struct Point3D {
    double x, y, z;
    int cluster = UNCLASSIFIED;
   
};



double euclidean_distance(const Point3D& a, const Point3D& b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

double euclidean_distance_sqr(const Point3D& a, const Point3D& b) {
    return pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2);
}


bool expand_cluster(std::vector<Point3D>& points, int point_id, int cluster, double eps, int min_pts, kdtree* tree) {
    std::vector<int> seeds;
    const double epsSquared = eps * eps; // Precompute eps squared

    #if KDTREE
    std::vector<double> query = {points[point_id].x, points[point_id].y, points[point_id].z};
    kdres *result;
    result = kd_nearest_range(tree, &query[0], epsSquared);;
    
    if (result == nullptr) {
        return false;
    }

    for (int i = 0; i < kd_res_size(result); ++i) {
        seeds.push_back((int) kd_res_item(result, i));
    }
    kd_res_free(result);
    #else    
    for (auto  it = points.begin(); it != points.end(); ++it) {
        if (euclidean_distance_sqr(points[point_id], *it) < epsSquared) {
            seeds.push_back(std::distance(points.begin(), it));
        }
    }
    #endif

    if (seeds.size() < min_pts) {
        points[point_id].cluster = NOISE;
        return false;
    }

    // Assign the cluster id to seeds
    for (int i = 0; i < seeds.size(); i++) {
        points[seeds[i]].cluster = cluster;
    }

    // Remove the original point from seeds
    seeds.erase(std::remove(seeds.begin(), seeds.end(), point_id), seeds.end());

    // Process every seed point
    while (!seeds.empty()) {
        int current_point = seeds.front();
        seeds.erase(seeds.begin());

        std::vector<int> result;
        for (int i = 0; i < points.size(); i++) {
            if (euclidean_distance_sqr(points[current_point], points[i]) < epsSquared) {
                result.push_back(i);
            }
        }

        if (result.size() >= min_pts) {
            for (int i = 0; i < result.size(); i++) {
                int result_point = result[i];
                if (points[result_point].cluster == UNCLASSIFIED || points[result_point].cluster == NOISE) {
                    if (points[result_point].cluster == UNCLASSIFIED) {
                        seeds.push_back(result_point);
                    }
                    points[result_point].cluster = cluster;
                }
            }
        }
    }
    return true;
}



void dbscan(std::vector<Point3D>& points, double eps, int min_pts) {
    int cluster = 1;
    
    #if KDTREE
    kdtree* tree = kd_create(3); // Build the KD-tree
    //populate kdtree
    for (size_t i = 0; i < points.size(); i++) {
        double pos[3] = {points[i].x, points[i].y, points[i].z};
        kd_insert(tree, pos, (void*)&points[i]);
    }

    #endif
    for (int i = 0; i < points.size(); i++) {
        if (points[i].cluster == UNCLASSIFIED) {
            #if KDTREE
            if (expand_cluster(points, i, cluster, eps, min_pts, tree)) {
                cluster++;
            }
            #else
            if (expand_cluster(points, i, cluster, eps, min_pts, nullptr)) {
                cluster++;
            }
            #endif
        }
    }
}


std::vector<Point3D> read_points_from_csv(const std::string& filename) {
    std::vector<Point3D> points;
    std::ifstream file(filename, std::ios::binary); // Open in binary mode to speed up reading
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return points; // Return an empty vector if the file cannot be opened
    }

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Skip the first line (header)
    
    std::string line;
    while (std::getline(file, line)) {
        Point3D point;
  

        char* end; // Temporary buffer for parsing numbers
    
        point.x = std::strtod(line.c_str(), &end);
        point.y = std::strtod(end + 1, &end);
        point.z = std::strtod(end + 1, nullptr);
       
        points.push_back(point);
    }

    return points;
}



void write_points_to_csv(const std::string& filename, const std::vector<Point3D>& points) {
    std::ofstream file(filename);
    // Check if the file stream is open and ready.
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    // Optional: Write the header
    file << "x,y,z,cluster\n";

    // Iterate over the points and write them to the file
    for (const auto& point : points) {
        file << point.x << "," << point.y << "," << point.z << "," << point.cluster << "\n";
    }
}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_filename>" << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];

    // Generate output filename based on input filename
    std::size_t dotPos = input_filename.rfind('.');
    std::string baseName = (dotPos == std::string::npos) ? input_filename : input_filename.substr(0, dotPos);
    std::string extension = (dotPos == std::string::npos) ? "" : input_filename.substr(dotPos);
    
    // Replace "INPUTS" with "OUTPUTS" in the baseName
    std::size_t inputsPos = baseName.find("INPUTS");
    if (inputsPos != std::string::npos) baseName.replace(inputsPos, strlen("INPUTS"), "OUTPUTS");
    std::string output_filename = baseName + "_clusters" + extension;

    // Read points from CSV
    std::vector<Point3D> points = read_points_from_csv(input_filename);

    // Parameters for DBSCAN
    double eps = 2.0; // Adjust based on your dataset
    int min_pts = 2; // Adjust based on your dataset

    auto start = std::chrono::high_resolution_clock::now(); // Before calling dbscan, get the starting time_point
    dbscan(points, eps, min_pts); // Apply DBSCAN
    auto stop = std::chrono::high_resolution_clock::now(); // After dbscan completes, get the ending time_point
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    
    std::cout << "BASIC_DBSCAN execution time: " << duration.count() << " milliseconds" << std::endl;
    
    // Write the clustered points to CSV
    write_points_to_csv(output_filename, points);
    std::cout << "Clustering results have been written to " << output_filename << std::endl;
    std::cout << "\n" << std::endl;
    
    return 0;
}


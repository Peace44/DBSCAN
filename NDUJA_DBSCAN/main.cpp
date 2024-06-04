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
#include "dbscan.h"

const int UNCLASSIFIED = 0;



std::vector<Point> read_points_from_csv(const std::string& filename) {
    std::vector<Point> points;
    std::ifstream file(filename, std::ios::binary); // Open in binary mode to speed up reading
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return points; // Return an empty vector if the file cannot be opened
    }

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Skip the first line (header)
    
    std::string line;
    while (std::getline(file, line)) {
        Point point;

        char* end; // Temporary buffer for parsing numbers

        point.x = std::strtod(line.c_str(), &end);
        point.y = std::strtod(end + 1, &end);
        point.z = std::strtod(end + 1, nullptr);

        points.push_back(point);
    }

    return points;
}



//void write_points_to_csv(const std::string& filename, const std::vector<std::vector<int>>& clusters) {
 void write_points_to_csv(const std::string& filename, const std::vector<Point>& points) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    file << "x,y,z,clusterID\n";
    
    for (const auto& point : points) {
        file << point.x << "," << point.y << "," << point.z << "," << point.clusterID << "\n";
    }
   


    // for (size_t i = 0; i < clusters.size(); ++i) {
    //     file << "Cluster " << i << ":\n";
    //     for (const auto& point : clusters[i]) {
    //         file << point << "\n";
    //     }
    //     file << "\n";
    // }

    file.close();
}



int main(int argc, char *argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <input_filename> <eps> <min_pts>" << std::endl;
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
    std::vector<Point> points = read_points_from_csv(input_filename);

    // Parameters for DBSCAN
    double eps = std::atoi(argv[2]); // Adjust based on your dataset
    int min_pts = std::atoi(argv[3]); // Adjust based on your dataset

    // Apply DBSCAN
    DBSCAN dbscan(eps, min_pts, 0.0, 0.0, 0.0, 0.0, points);
    auto start = std::chrono::high_resolution_clock::now(); // Before calling dbscan, get the starting time_point
    dbscan.run();    
    auto stop = std::chrono::high_resolution_clock::now(); // After dbscan completes, get the ending time_point
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    auto clusters = dbscan.getCluster(); 
    std::cout << "NDUJA'S DBSCAN execution time: " << duration.count() << " milliseconds" << std::endl;
    
     //transform clusters in array of points and write them to file
    for (const auto& cluster : clusters) {
        for (const auto& point : cluster) {
            points[point].clusterID = cluster[0];
        }
    }


    // Write the clustered points to CSV
//    write_points_to_csv(output_filename, clusters);
    
    write_points_to_csv(output_filename, points);
    // std::cout << "Clustering results have been written to " << output_filename << std::endl;
    
    return 0;
}

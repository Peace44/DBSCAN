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
#include <unordered_set>


const int NOISE = -1;
const int UNCLASSIFIED = 0;



struct Point3D {
    double x, y, z;
    int cluster = UNCLASSIFIED;
};



double euclideanDistance(const Point3D& a, const Point3D& b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}



// Function to find neighbors within ε distance
std::vector<int> regionQuery(const std::vector<Point3D>& points, int pIdx, double eps) {
    std::vector<int> neighbors;
    for (size_t i = 0; i < points.size(); ++i) {
        if (euclideanDistance(points[pIdx], points[i]) <= eps) {
            neighbors.push_back(i);
        }
    }
    return neighbors;
}



// Core-point check function
bool isCorePoint(const std::vector<Point3D>& points, int idx, double eps, int minPts) {
    return regionQuery(points, idx, eps).size() >= minPts;
}



void dbscanPlusPlus(std::vector<Point3D>& points, double eps, int minPts, int m) {
    // Random sampling of m points (simplified)
    std::random_shuffle(points.begin(), points.end());
    std::vector<Point3D> samplePoints(points.begin(), points.begin() + std::min(m, (int)points.size()));

    // Identify core points and construct the graph (simplified representation)
    std::unordered_set<int> corePointIndices;
    for (int i = 0; i < samplePoints.size(); ++i) {
        if (isCorePoint(points, i, eps, minPts)) {
            corePointIndices.insert(i);
        }
    }

    // Naive approach to cluster formation based on core points (for demonstration)
    int clusterId = 1;
    for (auto idx : corePointIndices) {
        if (points[idx].cluster == UNCLASSIFIED) {
            std::vector<int> neighbors = regionQuery(points, idx, eps);
            if (neighbors.size() >= minPts) {
                for (int neighborIdx : neighbors) {
                    points[neighborIdx].cluster = clusterId;
                }
                ++clusterId;
            }
        }
    }

    // Assign non-core points to clusters or mark as noise (simplified)
    for (size_t i = 0; i < points.size(); ++i) {
        if (points[i].cluster == UNCLASSIFIED) {
            points[i].cluster = NOISE;
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
    int m = 100; // Number of points to sample for DBSCAN++

    auto start = std::chrono::high_resolution_clock::now(); // Before calling dbscan, get the starting time_point
    dbscanPlusPlus(points, eps, min_pts, m); // Apply DBSCAN++
    auto stop = std::chrono::high_resolution_clock::now(); // After dbscan completes, get the ending time_point
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    
    std::cout << "BASIC_DBSCAN++ execution time: " << duration.count() << " milliseconds" << std::endl;
    
    // Write the clustered points to CSV
    write_points_to_csv(output_filename, points);
    std::cout << "Clustering results have been written to " << output_filename << std::endl;
    std::cout << "\n" << std::endl;

    return 0;
}


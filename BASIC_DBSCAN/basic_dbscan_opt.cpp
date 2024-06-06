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



const int NOISE = -1;
const int UNCLASSIFIED = 0;



struct Point3D {
    double x, y, z;
    int cluster = UNCLASSIFIED;
};



// 1-norm
double manhattan_distance(const Point3D& a, const Point3D& b)
{
    double _ax_bx_ = std::abs(a.x - b.x);
    double _ay_by_ = std::abs(a.y - b.y);
    double _az_bz_ = std::abs(a.z - b.z);

    return _ax_bx_ + _ay_by_ + _az_bz_;
}

// 2-norm
double euclidean_distance_sqrd(const Point3D& a, const Point3D& b) 
{
    double _ax_bx_ = a.x - b.x; // no need to calculate abs here
    double _ay_by_ = a.y - b.y; // no need to calculate abs here
    double _az_bz_ = a.z - b.z; // no need to calculate abs here

    return (_ax_bx_ * _ax_bx_) + (_ay_by_ * _ay_by_) + (_az_bz_ * _az_bz_);
}

// Infinity-norm
double chebyshev_distance(const Point3D& a, const Point3D& b)
{
    double _ax_bx_ = std::abs(a.x - b.x);
    double _ay_by_ = std::abs(a.y - b.y);
    double _az_bz_ = std::abs(a.z - b.z);
    
    return (_ax_bx_ >= _ay_by_) ? ((_ax_bx_ >= _az_bz_) ? _ax_bx_ : _az_bz_) : ((_ay_by_ >= _az_bz_) ? _ay_by_ : _az_bz_); // this returns max(_ax_bx_, _ay_by_, _az_bz_) very efficiently
}

using distance_function = double(*)(const Point3D&, const Point3D&);

bool expand_cluster(std::vector<Point3D>& points, int point_id, int cluster, double eps, int min_pts, distance_function dist_func) {
    std::vector<int> seeds;

    if (dist_func == euclidean_distance_sqrd) eps *= eps;

    for (auto  it = points.begin(); it != points.end(); ++it) {
        if (dist_func(points[point_id], *it) < eps) {
            seeds.push_back(std::distance(points.begin(), it));
        }
    }

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
        
        for (auto  it = points.begin(); it != points.end(); ++it) {
            if (dist_func(points[current_point], *it) < eps) {
                result.push_back(std::distance(points.begin(), it));
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



void dbscan(std::vector<Point3D>& points, double eps, int min_pts, distance_function dist_func) {
    int cluster = 1;
    for (int i = 0; i < points.size(); i++) {
        if (points[i].cluster == UNCLASSIFIED) {
            if (expand_cluster(points, i, cluster, eps, min_pts, dist_func)) {
                cluster++;
            }
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
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <input_filename> <eps> <min_pts> <norm_type>" << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];
    std::string output_filename = "BASIC_DBSCAN/basic_dbscan_opt.csv";

    // Read points from CSV
    std::vector<Point3D> points = read_points_from_csv(input_filename);

    // Parameters for DBSCAN
    double eps = std::atoi(argv[2]); // Adjust based on your dataset
    int min_pts = std::atoi(argv[3]); // Adjust based on your dataset

    // Parameter for the distance function
    std::string norm_type = argv[4];

    distance_function dist_func = nullptr;
    if (norm_type == "1") dist_func = manhattan_distance;
    else if (norm_type == "2") dist_func = euclidean_distance_sqrd;
    else if (norm_type == "inf") dist_func = chebyshev_distance;
    else std::cerr << "Unsupported norm_type. Use '1' for Manhattan (1-norm), '2' for Euclidean (2-norm), 'inf' for Chebyshev (inf-norm)" << std::endl;

    auto start = std::chrono::high_resolution_clock::now(); // Before calling dbscan, get the starting time_point
    dbscan(points, eps, min_pts, dist_func); // Apply DBSCAN
    auto stop = std::chrono::high_resolution_clock::now(); // After dbscan completes, get the ending time_point
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    
    std::cout << "BASIC_DBSCAN_OPT execution time: " << duration.count() << " microseconds" << std::endl;
    
    // Write the clustered points to CSV
    write_points_to_csv(output_filename, points);
    // std::cout << "Clustering results have been written to " << output_filename << std::endl;
    
    return 0;
}


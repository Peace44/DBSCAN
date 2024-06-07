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
#include <random>
#include <limits>



const int NOISE = -2;
const int UNCLASSIFIED = -1;



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

std::vector<int> initialize_core_points(std::vector<Point3D>& points, int m, distance_function dist_func) {
    std::vector<int> core_point_indices;
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, points.size() - 1);

    int first_index = distr(eng);
    core_point_indices.push_back(first_index);  // Start with a random point

    while (core_point_indices.size() < m) {
        double max_dist = -1;
        int farthest_idx = -1;

        for (int i = 0; i < points.size(); ++i) {
            double min_dist_to_core = std::numeric_limits<double>::max();
            for (int core_idx : core_point_indices) {
                double dist = dist_func(points[i], points[core_idx]);
                if (dist < min_dist_to_core) {
                    min_dist_to_core = dist;
                }
            }
            if (min_dist_to_core > max_dist) {
                max_dist = min_dist_to_core;
                farthest_idx = i;
            }
        }
        if (farthest_idx != -1 && std::find(core_point_indices.begin(), core_point_indices.end(), farthest_idx) == core_point_indices.end()) {
            core_point_indices.push_back(farthest_idx);
        }
    }
    return core_point_indices;
}



bool expand_cluster(std::vector<Point3D>& points, int point_id, int cluster_id, double eps, int min_pts, const std::vector<int>& core_points, distance_function dist_func) {
    std::vector<int> seeds;

    for (int idx : core_points) {
        if (dist_func(points[point_id], points[idx]) < eps) {
            seeds.push_back(idx);
        }
    }

    if (seeds.size() < min_pts) {
        points[point_id].cluster = NOISE;
        return false;
    }

    for (int i : seeds) {
        points[i].cluster = cluster_id;
    }

    seeds.erase(std::remove(seeds.begin(), seeds.end(), point_id), seeds.end());

    while (!seeds.empty()) {
        int current_point = seeds.front();
        seeds.erase(seeds.begin());

        std::vector<int> result;
        for (int idx : core_points) {
            if (dist_func(points[current_point], points[idx]) < eps && (points[idx].cluster == UNCLASSIFIED || points[idx].cluster == NOISE)) {
                result.push_back(idx);
            }
        }

        if (result.size() >= min_pts) {
            for (int idx : result) {
                if (points[idx].cluster == UNCLASSIFIED || points[idx].cluster == NOISE) {
                    points[idx].cluster = cluster_id;
                    seeds.push_back(idx);
                }
            }
        }
    }
    return true;
}



void dbscan(std::vector<Point3D>& points, double eps, int min_pts, const std::vector<int>& core_points, distance_function dist_func) {
    int cluster_id = 1;  // Start with cluster 1
    for (int idx : core_points) {
        if (points[idx].cluster == UNCLASSIFIED) {
            if (expand_cluster(points, idx, cluster_id, eps, min_pts, core_points, dist_func)) {
                cluster_id++;
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
    std::string output_filename = "BASIC_DBSCAN++/basic_dbscan++.csv";

    // Read points from CSV
    std::vector<Point3D> points = read_points_from_csv(input_filename);

    // Parameters for DBSCAN
    double eps = std::atoi(argv[2]); // Adjust based on your dataset
    int min_pts = std::atoi(argv[3]); // Adjust based on your dataset
    
    // Parameter for the distance function
    std::string norm_type = argv[4];

    distance_function dist_func = nullptr;
    if (norm_type == "1") dist_func = manhattan_distance;
    else if (norm_type == "2") {dist_func = euclidean_distance_sqrd; eps *= eps;}
    else if (norm_type == "inf") dist_func = chebyshev_distance;
    else std::cerr << "Unsupported norm_type. Use '1' for Manhattan (1-norm), '2' for Euclidean (2-norm), 'inf' for Chebyshev (inf-norm)" << std::endl;

    int m = 10; // Number of core points to initialize

    std::vector<int> core_points = initialize_core_points(points, m, dist_func);
    
    auto start = std::chrono::high_resolution_clock::now(); // Before calling dbscan, get the starting time_point
    dbscan(points, eps, min_pts, core_points, dist_func);
    auto stop = std::chrono::high_resolution_clock::now(); // After dbscan completes, get the ending time_point
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    
    std::cout << "BASIC_DBSCAN++ execution time: " << duration.count() << " microseconds" << std::endl;

    // Write the clustered points to CSV
    write_points_to_csv(output_filename, points);
    // std::cout << "Clustering results have been written to " << output_filename << std::endl;

    return 0;
}

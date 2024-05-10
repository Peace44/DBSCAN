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

bool expand_cluster(std::vector<Point3D>& points, int point_id, int cluster, double eps, int min_pts, std::vector<int>& core_points) {
    std::vector<int> seeds;
    const double epsSquared = eps * eps; // Precompute eps squared

    // Process only core points for efficiency
    for (int idx : core_points) {
        if (euclidean_distance_sqr(points[point_id], points[idx]) < epsSquared) {
            seeds.push_back(idx);
        }
    }

    if (seeds.size() < min_pts) {
        points[point_id].cluster = NOISE;
        return false;
    }

    // Assign the cluster id to seeds
    for (int i : seeds) {
        points[i].cluster = cluster;
    }

    // Remove the original point from seeds
    seeds.erase(std::remove(seeds.begin(), seeds.end(), point_id), seeds.end());

    // Process every seed point
    while (!seeds.empty()) {
        int current_point = seeds.front();
        seeds.erase(seeds.begin());

        std::vector<int> result;
        
        for (int idx : core_points) {
            if (euclidean_distance_sqr(points[current_point], points[idx]) < epsSquared) {
                result.push_back(idx);
            }
        }

        if (result.size() >= min_pts) {
            for (int i : result) {
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

void dbscan(std::vector<Point3D>& points, double eps, int min_pts, const std::vector<int>& core_points) {
    int cluster = 1;
    for (int idx : core_points) {
        if (points[idx].cluster == UNCLASSIFIED) {
            if (expand_cluster(points, idx, cluster, eps, min_pts, core_points)) {
                cluster++;
            }
        }
    }
}

std::vector<Point3D> read_points_from_csv(const std::string& filename) {
    std::vector<Point3D> points;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Point3D point;
        ss >> point.x >> point.y >> point.z;
        points.push_back(point);
    }
    return points;
}

std::vector<int> initialize_core_points(std::vector<Point3D>& points, int m) {
    std::vector<int> core_point_indices;
    std::random_device rd; // Obtain a random number from hardware
    std::mt19937 eng(rd()); // Seed the generator
    std::uniform_int_distribution<> distr(0, points.size() - 1); // Define the range

    // Implementing a simple random selection for now, replace with K-center for actual use
    while (core_point_indices.size() < m) {
        int idx = distr(eng);
        if (std::find(core_point_indices.begin(), core_point_indices.end(), idx) == core_point_indices.end()) {
            core_point_indices.push_back(idx);
        }
    }
    return core_point_indices;
}

int main() {
    std::string filename = "points.csv";
    double eps = 0.5;
    int min_pts = 5;
    int m = 100; // Number of core points to initialize

    std::vector<Point3D> points = read_points_from_csv(filename);
    std::vector<int> core_points = initialize_core_points(points, m);

    dbscan(points, eps, min_pts, core_points);

    for (const auto& point : points) {
        std::cout << "Point(" << point.x << ", " << point.y << ", " << point.z << ") - Cluster: " << point.cluster << std::endl;
    }

    return 0;
}

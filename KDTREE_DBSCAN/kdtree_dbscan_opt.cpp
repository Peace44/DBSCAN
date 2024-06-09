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
#include <nanoflann.hpp> // sudo apt install libnanoflann-dev
#include <unistd.h>
#include <limits.h>



const int NOISE = -2;
const int UNCLASSIFIED = -1;



struct Point3D {
    double x, y, z;
    int cluster = UNCLASSIFIED;
};



// 1-norm
double manhattan_distance(const double _dx_, const double _dy_, const double _dz_)
{
    return _dx_ + _dy_ + _dz_;
}

// 2-norm
double euclidean_distance_sqrd(const double _dx_, const double _dy_, const double _dz_) 
{
    return (_dx_ * _dx_) + (_dy_ * _dy_) + (_dz_ * _dz_);
}

// Infinity-norm
double chebyshev_distance(const double _dx_, const double _dy_, const double _dz_)
{
    return std::max(std::max(_dx_, _dy_), _dz_);
}

using distance_function = double(*)(const double, const double, const double);

distance_function dist_func = nullptr;



// Adapt nanoflann to work with Point3D
struct PointCloud {
    std::vector<Point3D> points;

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const { return points.size(); }

    // Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class
    inline double kdtree_distance(const double* p1, const size_t idx_p2, size_t size) const {
        const double d0 = std::abs(p1[0] - points[idx_p2].x);
        const double d1 = std::abs(p1[1] - points[idx_p2].y);
        const double d2 = std::abs(p1[2] - points[idx_p2].z);
        // return d0 * d0 + d1 * d1 + d2 * d2;
        return dist_func(d0, d1, d2);
    }

    // Returns the dim'th component of the idx'th point in the class
    inline double kdtree_get_pt(const size_t idx, int dim) const {
        if (dim == 0) return points[idx].x;
        else if (dim == 1) return points[idx].y;
        else return points[idx].z;
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }
};

typedef nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<double, PointCloud>,
    PointCloud,
    3 /* dim */
> my_kd_tree_t;

void find_neighbors(const PointCloud& cloud, my_kd_tree_t& index, int point_id, double eps, std::vector<int>& neighbors) {
    const Point3D& query_point = cloud.points[point_id];
    std::vector<std::pair<unsigned int, double>> ret_matches;
    nanoflann::SearchParams params;
    const double query_pt[3] = { query_point.x, query_point.y, query_point.z };
    const size_t nMatches = index.radiusSearch(&query_pt[0], eps, ret_matches, params);

    neighbors.reserve(nMatches);
    for (size_t i = 0; i < nMatches; ++i) {
        neighbors.push_back(ret_matches[i].first);
    }
}

bool expand_cluster(PointCloud& cloud, my_kd_tree_t& index, int point_id, int cluster, double eps, int min_pts) {
    std::vector<int> seeds;
    
    find_neighbors(cloud, index, point_id, eps, seeds);

    if (seeds.size() < min_pts) {
        cloud.points[point_id].cluster = NOISE;
        return false;
    }

    for (int i : seeds) {
        cloud.points[i].cluster = cluster;
    }

    seeds.erase(std::remove(seeds.begin(), seeds.end(), point_id), seeds.end());

    while (!seeds.empty()) {
        int current_point = seeds.front();
        seeds.erase(seeds.begin());

        std::vector<int> result;
        find_neighbors(cloud, index, current_point, eps, result);

        if (result.size() >= min_pts) {
            for (int i : result) {
                if (cloud.points[i].cluster == UNCLASSIFIED || cloud.points[i].cluster == NOISE) {
                    if (cloud.points[i].cluster == UNCLASSIFIED) {
                        seeds.push_back(i);
                    }
                    cloud.points[i].cluster = cluster;
                }
            }
        }
    }
    return true;
}

void dbscan(PointCloud& cloud, double eps, int min_pts) {
    my_kd_tree_t index(3, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    index.buildIndex();

    int cluster = UNCLASSIFIED + 1;
    for (int i = 0; i < cloud.points.size(); i++) {
        if (cloud.points[i].cluster == UNCLASSIFIED) {
            if (expand_cluster(cloud, index, i, cluster, eps, min_pts)) {
                cluster++;
            }
        }
    }
}

std::vector<Point3D> read_points_from_csv(const std::string& filename) {
    std::vector<Point3D> points;
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return points;
    }

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    std::string line;
    while (std::getline(file, line)) {
        Point3D point;
        char* end;

        point.x = std::strtod(line.c_str(), &end);
        point.y = std::strtod(end + 1, &end);
        point.z = std::strtod(end + 1, nullptr);

        points.push_back(point);
    }

    return points;
}

void write_points_to_csv(const std::string& filename, const std::vector<Point3D>& points) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    file << "x,y,z,cluster\n";
    for (const auto& point : points) {
        file << point.x << "," << point.y << "," << point.z << "," << point.cluster << "\n";
    }
}



std::string getExecutablePath() {
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1) {
        throw std::runtime_error("Error getting executable path");
    }
    return std::string(path, count);
}



int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <input_filename> <eps> <min_pts> <norm_type>" << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];
    std::string output_filename = getExecutablePath() + ".csv";

    std::vector<Point3D> points = read_points_from_csv(input_filename);

    double eps = std::stod(argv[2]);
    int min_pts = std::stoi(argv[3]);

    std::string norm_type = argv[4];

    if (norm_type == "1") dist_func = manhattan_distance;
    else if (norm_type == "2") {dist_func = euclidean_distance_sqrd; eps *= eps;}
    else if (norm_type == "inf") dist_func = chebyshev_distance;
    else std::cerr << "Unsupported norm_type. Use '1' for Manhattan (1-norm), '2' for Euclidean (2-norm), 'inf' for Chebyshev (inf-norm)" << std::endl;

    PointCloud cloud;
    cloud.points = points;

    auto start = std::chrono::high_resolution_clock::now();
    dbscan(cloud, eps, min_pts);
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << duration.count() << std::endl;

    write_points_to_csv(output_filename, cloud.points);
    // std::cout << "Clustering results have been written to " << output_filename << std::endl;

    return 0;
}

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



// ----- Distance Functor Structs -----
// Define functor structs for each distance metric, marked as inline.
struct Manhattan {
    inline double operator()(double dx, double dy, double dz) const {
        return dx + dy + dz;
    }

    // For Manhattan, accumulation is the absolute difference
    inline double accum_dist(const double a, const double b, int /*dim*/) const {
        return std::abs(a - b);
    }
};

struct EuclideanSquared {
    inline double operator()(double dx, double dy, double dz) const {
        return dx * dx + dy * dy + dz * dz;
    }

    // For Euclidean squared distance, use squared difference
    inline double accum_dist(const double a, const double b, int /*dim*/) const {
        double d = a - b;
        return d * d;
    }
};

struct Chebyshev {
    inline double operator()(double dx, double dy, double dz) const {
        return std::max({dx, dy, dz});
    }

    // For Chebyshev, we return the absolute difference.
    // (Note: kd-tree bounding box computations sum these values,
    // so this may not yield exact Chebyshev behavior without further modification.
    // If you run into further issues with Chebyshev (since its true metric is not additive), 
    // you might consider either not using it for kd‑tree searches or writing a custom kd‑tree search routine tailored to the maximum norm.)
    inline double accum_dist(const double a, const double b, int /*dim*/) const {
        return std::abs(a - b);
    }
};


// Adapt nanoflann to work with Point3D
// ----- Templated PointCloud Structure -----
// This structure stores points and a distance functor.
template <typename DistanceFunctor>
struct TPointCloud {
    std::vector<Point3D> points;
    DistanceFunctor distance;  // Functor instance

    // Returns the number of points
    inline size_t kdtree_get_point_count() const { 
        return points.size(); 
    }

    // Computes the distance between a query point (p1) and the point at index idx_p2
    inline double kdtree_distance(const double* p1, const size_t idx_p2, size_t /*size*/) const {
        const double d0 = std::abs(p1[0] - points[idx_p2].x);
        const double d1 = std::abs(p1[1] - points[idx_p2].y);
        const double d2 = std::abs(p1[2] - points[idx_p2].z);
        return distance(d0, d1, d2);
    }

    // Returns the dim-th coordinate of the point at index idx.
    inline double kdtree_get_pt(const size_t idx, int dim) const {
        if (dim == 0) return points[idx].x;
        else if (dim == 1) return points[idx].y;
        else return points[idx].z;
    }

    // Optional bounding-box computation: return false to use the default.
    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /*bb*/) const { 
        return false; 
    }
};



// ----- Custom Distance Adaptor for nanoflann -----
// This adaptor wraps our templated point cloud's distance function.
template <class T, class DataSource>
struct CustomDistanceAdaptor {
    typedef T ElementType;
    typedef T ResultType;
    typedef T DistanceType;
    
    const DataSource &data_source;

    CustomDistanceAdaptor(const DataSource &ds) : data_source(ds) {}

    inline ResultType evalMetric(const T* a, const size_t b_idx, size_t size) const {
        return data_source.kdtree_distance(a, b_idx, size);
    }

    // Forward accum_dist to the underlying distance functor stored in data_source.
    inline T accum_dist(const T a, const T b, int dim) const {
        return data_source.distance.accum_dist(a, b, dim);
    }
};


// ----- DBSCAN Functions -----

// Finds neighbors of point 'point_id' within radius 'eps'
template <typename PointCloudType, typename KDTreeType>
void find_neighbors(const PointCloudType& cloud, KDTreeType& index, int point_id, double eps, std::vector<int>& neighbors) {
    const Point3D& query_point = cloud.points[point_id];
    std::vector<std::pair<int, double>> ret_matches;
    nanoflann::SearchParams params;
    const double query_pt[3] = { query_point.x, query_point.y, query_point.z };
    const size_t nMatches = index.radiusSearch(&query_pt[0], eps, ret_matches, params);
    
    neighbors.clear();
    neighbors.reserve(nMatches);
    for (size_t i = 0; i < nMatches; ++i) neighbors.push_back(ret_matches[i].first);
}

// Expands the cluster from a seed point. Returns true if a new cluster is formed.
template <typename PointCloudType, typename KDTreeType>
bool expand_cluster(PointCloudType& cloud, KDTreeType& index, int point_id, int cluster, double eps, int min_pts) {
    std::vector<int> seeds;
    find_neighbors(cloud, index, point_id, eps, seeds);
    if (seeds.size() < static_cast<size_t>(min_pts)) {
        cloud.points[point_id].cluster = NOISE;
        return false;
    }

    for (int i : seeds) cloud.points[i].cluster = cluster; // Assign the cluster label to all seeds

    // Remove the initial seed point from the list
    seeds.erase(std::remove(seeds.begin(), seeds.end(), point_id), seeds.end());
    // Process the seeds
    while (!seeds.empty()) {
        int current_point = seeds.front();
        seeds.erase(seeds.begin());
        std::vector<int> result;
        find_neighbors(cloud, index, current_point, eps, result);
        if (result.size() >= static_cast<size_t>(min_pts)) {
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

// DBSCAN that uses an already built kd-tree index.
template <typename PointCloudType, typename KDTreeType>
void dbscan_with_index(PointCloudType& cloud, KDTreeType& index, double eps, int min_pts) {
    int cluster = UNCLASSIFIED + 1;
    for (int i = 0; i < cloud.points.size(); i++) {
        if (cloud.points[i].cluster == UNCLASSIFIED) {
            if (expand_cluster(cloud, index, i, cluster, eps, min_pts)) {
                cluster++;
            }
        }
    }
}



// ----- CSV Input/Output Functions -----
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

    // Depending on the norm_type, instantiate the proper templated point cloud and kd-tree.
    if (norm_type == "1") {
        TPointCloud<Manhattan> cloud;
        cloud.points = points;
        cloud.distance = Manhattan();

        typedef nanoflann::KDTreeSingleIndexAdaptor<
            CustomDistanceAdaptor<double, TPointCloud<Manhattan>>,
            TPointCloud<Manhattan>, 3, int
        > my_kd_tree_t;

        my_kd_tree_t index(3, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10));
        index.buildIndex();

        auto start = std::chrono::high_resolution_clock::now();
        dbscan_with_index(cloud, index, eps, min_pts);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << std::endl;

        write_points_to_csv(output_filename, cloud.points);
    }
    else if (norm_type == "2") {
        TPointCloud<EuclideanSquared> cloud;
        cloud.points = points;
        cloud.distance = EuclideanSquared();
        eps *= eps;  // Adjust eps for squared Euclidean distance

        typedef nanoflann::KDTreeSingleIndexAdaptor<
            CustomDistanceAdaptor<double, TPointCloud<EuclideanSquared>>,
            TPointCloud<EuclideanSquared>, 3, int
        > my_kd_tree_t;

        my_kd_tree_t index(3, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10));
        index.buildIndex();

        auto start = std::chrono::high_resolution_clock::now();
        dbscan_with_index(cloud, index, eps, min_pts);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << std::endl;

        write_points_to_csv(output_filename, cloud.points);
    }
    else if (norm_type == "inf") {
        TPointCloud<Chebyshev> cloud;
        cloud.points = points;
        cloud.distance = Chebyshev();

        typedef nanoflann::KDTreeSingleIndexAdaptor<
            CustomDistanceAdaptor<double, TPointCloud<Chebyshev>>,
            TPointCloud<Chebyshev>, 3, int
        > my_kd_tree_t;

        my_kd_tree_t index(3, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10));
        index.buildIndex();

        auto start = std::chrono::high_resolution_clock::now();
        dbscan_with_index(cloud, index, eps, min_pts);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << std::endl;

        write_points_to_csv(output_filename, cloud.points);
    }
    else std::cerr << "Unsupported norm_type. Use '1' for Manhattan (1-norm), '2' for Euclidean (2-norm), 'inf' for Chebyshev (inf-norm)" << std::endl;
    


    return 0;
}

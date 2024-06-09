#include <mlpack/core.hpp>
#include <mlpack/methods/dbscan/dbscan.hpp>
#include <mlpack/core/metrics/lmetric.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>



const int NOISE = -2;
const int UNCLASSIFIED = -1;



struct Point3D {
    double x, y, z;
    int cluster = UNCLASSIFIED;
};



// Define the distance metrics using LMetric
using manhattan_distance = mlpack::metric::ManhattanDistance;
using euclidean_distance_sqrd = mlpack::metric::SquaredEuclideanDistance;
using chebyshev_distance = mlpack::metric::ChebyshevDistance;

template<typename MetricType>
void run_dbscan(const arma::mat& dataset, double eps, size_t min_pts, arma::Row<size_t>& assignments) {
    mlpack::dbscan::DBSCAN<mlpack::range::RangeSearch<MetricType>> dbscan(eps, min_pts);
    dbscan.Cluster(dataset, assignments);
}

void cluster_dataset(const arma::mat& dataset, double eps, size_t min_pts, const std::string& norm_type, arma::Row<size_t>& assignments)
{
    if (norm_type == "1") run_dbscan<manhattan_distance>(dataset, eps, min_pts, assignments);
    else if (norm_type == "2") run_dbscan<euclidean_distance_sqrd>(dataset, eps * eps, min_pts, assignments);
    else if (norm_type == "inf") run_dbscan<chebyshev_distance>(dataset, eps, min_pts, assignments);
    else throw std::invalid_argument("Unsupported norm_type. Use '1' for Manhattan (1-norm), '2' for Euclidean (2-norm), 'inf' for Chebyshev (inf-norm)");
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

    // Load the dataset
    arma::mat dataset;
    bool load_status = dataset.load(input_filename, arma::csv_ascii);
    if (!load_status) {
        std::cerr << "Error loading dataset. Please check the file path and format!" << std::endl;
        return -1;
    }
    dataset.shed_row(0); // Assuming the 1st row is header and should be ignored
    dataset = dataset.t(); // Transpose the dataset because it's loaded with each point as a row, and we need points as cols for mlpack

    // Parameters for DBSCAN
    double eps = std::atof(argv[2]); // Adjust based on your dataset
    int min_pts = std::atoi(argv[3]); // Adjust based on your dataset
    std::string norm_type = argv[4];

    // std::unique_ptr<DBSCANBase> dbscan = create_dbscan(eps, min_pts, norm_type);
    arma::Row<size_t> assignments; // This will hold the cluster assignment

    auto start = std::chrono::high_resolution_clock::now(); // Before calling dbscan, get the starting time_point
    cluster_dataset(dataset, eps, min_pts, norm_type, assignments);
    auto stop = std::chrono::high_resolution_clock::now(); // After dbscan completes, get the ending time_point
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    
    std::cout << duration.count() << std::endl;
    
    // Prepare a vector of Point3D objects
    std::vector<Point3D> points(dataset.n_cols);
    for (size_t i = 0; i < dataset.n_cols; i++) {
        points[i].x = dataset(0, i);
        points[i].y = dataset(1, i);
        points[i].z = dataset(2, i);
        points[i].cluster = (assignments[i] == size_t(-1)) ? NOISE : static_cast<int>(assignments[i]) + 1;
    }

    // Write the clustered points to CSV
    write_points_to_csv(output_filename, points);
    // std::cout << "Clustering results have been written to " << output_filename << std::endl;

    return 0;
}


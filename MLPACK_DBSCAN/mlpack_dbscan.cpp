#include <mlpack/core.hpp>
#include <mlpack/methods/dbscan/dbscan.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>



const int NOISE = -1;
const int UNCLASSIFIED = 0;



struct Point3D {
    double x, y, z;
    int cluster = UNCLASSIFIED;
};



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
    double eps = 2.0; // Adjust based on your dataset
    int min_pts = 5; // Adjust based on your dataset

    mlpack::dbscan::DBSCAN<> dbscan(eps, min_pts);
    arma::Row<size_t> assignments; // This will hold the cluster assignment

    auto start = std::chrono::high_resolution_clock::now(); // Before calling dbscan, get the starting time_point
    dbscan.Cluster(dataset, assignments); // Apply DBSCAN
    auto stop = std::chrono::high_resolution_clock::now(); // After dbscan completes, get the ending time_point
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    
    std::cout << "MLPACK_DBSCAN execution time: " << duration.count() << " milliseconds" << std::endl;
    
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
    std::cout << "Clustering results have been written to " << output_filename << std::endl;
    std::cout << "\n" << std::endl;

    return 0;
}


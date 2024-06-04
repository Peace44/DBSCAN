#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <fstream>

struct Point
{
    std::vector<double> coords;
    bool isCore;
    int cluster;

    Point(std::vector<double> c) : coords(c), isCore(false), cluster(-1) {}
};

double euclideanDistance(const Point &p1, const Point &p2)
{
    double sum = 0;
    for (size_t i = 0; i < p1.coords.size(); ++i)
    {
        sum += std::pow(p1.coords[i] - p2.coords[i], 2);
    }
    return std::sqrt(sum);
}

std::vector<Point> uniformSample(const std::vector<Point> &data, size_t m)
{
    std::vector<Point> sample;
    std::unordered_set<int> indices;
    while (sample.size() < m)
    {
        int index = rand() % data.size();
        if (indices.find(index) == indices.end())
        {
            sample.push_back(data[index]);
            indices.insert(index);
        }
    }
    return sample;
}

std::vector<Point> kCenterSample(const std::vector<Point> &data, size_t m)
{
    std::vector<Point> sample;
    sample.push_back(data[0]);
    while (sample.size() < m)
    {
        Point furthest = data[0];
        double maxDist = 0;
        for (const auto &point : data)
        {
            double minDist = std::numeric_limits<double>::max();
            for (const auto &s : sample)
            {
                double dist = euclideanDistance(point, s);
                minDist = std::min(minDist, dist);
            }
            if (minDist > maxDist)
            {
                maxDist = minDist;
                furthest = point;
            }
        }
        sample.push_back(furthest);
    }
    return sample;
}

std::vector<Point> findCorePoints(const std::vector<Point> &data, const std::vector<Point> &sample, double epsilon, int minPts)
{
    std::vector<Point> corePoints = sample;
    for (auto &point : corePoints)
    {
        int count = 0;
        for (const auto &p : data)
        {
            if (euclideanDistance(point, p) <= epsilon)
            {
                ++count;
            }
        }
        point.isCore = (count >= minPts);
    }
    return corePoints;
}

std::unordered_map<int, std::vector<Point>> findClusters(std::vector<Point> &corePoints, double epsilon)
{
    std::unordered_map<int, std::vector<Point>> clusters;
    int clusterId = 0;
    for (auto &point : corePoints)
    {
        if (point.cluster == -1 && point.isCore)
        {
            std::queue<Point *> q;
            q.push(&point);
            point.cluster = clusterId;
            while (!q.empty())
            {
                Point *p = q.front();
                q.pop();
                for (auto &cp : corePoints)
                {
                    if (cp.cluster == -1 && cp.isCore && euclideanDistance(*p, cp) <= epsilon)
                    {
                        cp.cluster = clusterId;
                        q.push(&cp);
                    }
                }
            }
            ++clusterId;
        }
    }
    return clusters;
}

void assignRemainingPoints(std::vector<Point> &data, const std::unordered_map<int, std::vector<Point>> &clusters, double epsilon)
{
    for (auto &point : data)
    {
        if (point.cluster == -1)
        {
            double minDist = std::numeric_limits<double>::max();
            int closestCluster = -1;
            for (const auto &[clusterId, corePoints] : clusters)
            {
                for (const auto &corePoint : corePoints)
                {
                    double dist = euclideanDistance(point, corePoint);
                    if (dist < minDist)
                    {
                        minDist = dist;
                        closestCluster = clusterId;
                    }
                }
            }
            if (minDist <= epsilon)
            {
                point.cluster = closestCluster;
            }
        }
    }
}

std::vector<Point> read_points_from_csv(const std::string &filename)
{
    std::vector<Point> points;
    std::ifstream file(filename, std::ios::binary); // Open in binary mode to speed up reading
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return points; // Return an empty vector if the file cannot be opened
    }

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Skip the first line (header)

    std::string line;
    while (std::getline(file, line))
    {
        Point point({0.0, 0.0, 0.0});
        point.cluster = -1;

        char *end; // Temporary buffer for parsing numbers

        point.coords[0] = std::strtod(line.c_str(), &end);
        point.coords[1] = std::strtod(end + 1, &end);
        point.coords[2] = std::strtod(end + 1, nullptr);

        points.push_back(point);
    }

    return points;
}

int main(int argc, char *argv[])
{

    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " <input_filename> <eps> <min_pts>" << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];

    // std::vector<Point> data = read_points_from_csv(input_filename); // Load your dataset here
    std::vector<Point> data = {
        Point({1.0, 2.0}),
        Point({1.1, 2.1}),
        Point({0.9, 1.8}),
        Point({8.0, 8.0}),
        Point({8.1, 8.1}),
        Point({7.9, 7.8})};

    // size_t m = 400; // Number of points to sample
    // double epsilon = 2.0;
    // int minPts = 2;
    size_t m = 3; // Number of points to sample
    double epsilon = 0.5;
    int minPts = 2;
    std::vector<Point> sample = uniformSample(data, m);
    std::vector<Point> corePoints = findCorePoints(data, sample, epsilon, minPts);
    std::unordered_map<int, std::vector<Point>> clusters = findClusters(corePoints, epsilon);
    assignRemainingPoints(data, clusters, epsilon);

    // Print results
    for (const auto &point : data)
    {
        std::cout << "Point: ";
        for (const auto &coord : point.coords)
        {
            std::cout << coord << " ";
        }
        std::cout << "Cluster: " << point.cluster << std::endl;
    }

    return 0;
}

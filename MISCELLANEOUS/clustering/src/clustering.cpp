#include "Eigen/Core"
#include <map>
#include <pcl/common/transforms.h>
#include <pcl/io/ply_io.h>
#include <pcl_conversions/pcl_conversions.h>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <std_msgs/msg/header.hpp>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
#include <tf2_eigen/tf2_eigen.hpp>
#include <std_msgs/msg/bool.hpp>
#include <fstream>
#include <iostream>

#include "rclcpp/rclcpp.hpp"
#include "rmw/qos_profiles.h"
#include "rclcpp/qos.hpp"

#include "ground_segmentation/ground_segmentation.h"
#include "dbscan/dbscan.h"


using std::placeholders::_1;
using namespace std::chrono_literals;



void write_points_to_csv(const std::string& filename, std::vector<pcl::PointXYZ>& points) {
    std::ofstream file(filename);
    // Check if the file stream is open and ready.
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    // Optional: Write the header
    file << "x,y,z\n";

    // Iterate over the points and write them to the file
    for (const auto& point : points) {
        file << point.x << "," << point.y << "," << point.z << "\n";
    }
}



class Clustering : public rclcpp::Node
{
  public:
    Clustering()
    : Node("clustering")
    {
      // Ground condition
      declare_parameter("vehicle_hiding_len", 1.8);
      declare_parameter("vehicle_hiding_width", 1.6);
      declare_parameter("cone_height", 1.);
      declare_parameter("sensor_inclination", 0.);
      declare_parameter("sensor_height", 1.2);
      declare_parameter("x_filter", 0.5);
      declare_parameter("max_dist_to_line", 0.5);
      declare_parameter("max_slope", 0.3);
      declare_parameter("min_slope", 0.);
      declare_parameter("max_fit_error", 0.1);
      declare_parameter("max_start_height", 0.5);
      declare_parameter("long_threshold", 1.);
      declare_parameter("max_long_height", 0.1);
      declare_parameter("line_search_angle", 0.1);
      
      // Segmentation
      declare_parameter("r_min", 0.5);
      declare_parameter("r_max", 50.);
      declare_parameter("n_bins", 120);
      declare_parameter("n_segments", 360);
      
      // Other
      declare_parameter("n_threads", 4);

      // dbscan
      declare_parameter("eps", 0.2);
      declare_parameter("min_points", 2);
      declare_parameter("centroide_max_distance", 0.35);

      declare_parameter("plot", false);
      declare_parameter("debug", false);


      _vehicle_hiding_len = get_parameter("vehicle_hiding_len").as_double();
      _vehicle_hiding_width = get_parameter("vehicle_hiding_width").as_double();
      _cone_height = get_parameter("cone_height").as_double();
      _sensor_inclination = get_parameter("sensor_inclination").as_double()*M_PI/180;
      _params.visualize = false;
      _params.n_bins = get_parameter("n_bins").as_int();
      _params.n_segments = get_parameter("n_segments").as_int();
      _max_dist_to_line = get_parameter("max_dist_to_line").as_double();
      _params.max_dist_to_line = _max_dist_to_line;
      _params.max_slope = get_parameter("max_slope").as_double();
      _params.min_slope = get_parameter("min_slope").as_double();
      _params.long_threshold = get_parameter("long_threshold").as_double();
      _params.max_long_height = get_parameter("max_long_height").as_double();
      _params.max_start_height = get_parameter("max_start_height").as_double();
      _sensor_height = get_parameter("sensor_height").as_double();
      _params.sensor_height = _sensor_height;
      _x_filter = get_parameter("x_filter").as_double();
      _params.line_search_angle = get_parameter("line_search_angle").as_double();
      _params.n_threads = get_parameter("n_threads").as_int();
      _r_min = get_parameter("r_min").as_double();
      _r_max = get_parameter("r_max").as_double();
      _params.r_min_square = std::pow(_r_min, 2);
      _params.r_max_square = std::pow(_r_max, 2);
      _params.max_error_square = std::pow(get_parameter("max_fit_error").as_double(), 2);

      _eps = get_parameter("eps").as_double();
      _min_points = get_parameter("min_points").as_int();
      _centroide_max_distance = get_parameter("centroide_max_distance").as_double();

      _plot = get_parameter("plot").as_bool();
      _debug = get_parameter("debug").as_bool();

      _debug = true;


      _tf_buffer = std::make_unique<tf2_ros::Buffer>(this->get_clock());
      _tf_listener = std::make_shared<tf2_ros::TransformListener>(*_tf_buffer);

      _segmenter.init(_params);

      // Reliability policy
      // rmw_qos_profile_t qos_profile = rmw_qos_profile_sensor_data;
      // rclcpp::QoS qos = rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(qos_profile), qos_profile);

      // _point_cloud_subscription = this->create_subscription<sensor_msgs::msg::PointCloud2>(
      //   "/ouster/points", qos, std::bind(&Clustering::point_cloud_callback, this, _1));
    
      _point_cloud_subscription = this->create_subscription<sensor_msgs::msg::PointCloud2>(
        "/ouster/points", 1, std::bind(&Clustering::point_cloud_callback, this, std::placeholders::_1));

      _sync_trigger_publisher = this->create_publisher<std_msgs::msg::Bool>("sync_trigger", 1);

      if(_plot)
      {
        _point_cloud_plot_publisher = this->create_publisher<sensor_msgs::msg::PointCloud2>("point_cloud_rviz", 1);
        _obstacle_cloud_plot_publisher = this->create_publisher<sensor_msgs::msg::PointCloud2>("obstacle_cloud_rviz", 1);
        _centroid_cloud_plot_publisher = this->create_publisher<sensor_msgs::msg::PointCloud2>("centroid_cloud_rviz", 1);
      }
      _centroid_cloud_publisher = this->create_publisher<sensor_msgs::msg::PointCloud2>("centroid_cloud", 1);
    }

  private:

    void point_cloud_callback(const sensor_msgs::msg::PointCloud2 & point_cloud_){    
      std_msgs::msg::Bool bool_msg;
      _sync_trigger_publisher->publish(bool_msg);

      // if(_debug)
      // {
      //   static rclcpp::Time time_old = this->now();
      //   rclcpp::Time time_now = this->now();
      //   double ct = (time_now-time_old).seconds()*1000;
      //   RCLCPP_INFO_STREAM(this->get_logger(), "Rate: " << ct << " ms");
      //   time_old = time_now;
      // }


      rclcpp::Time start = this->now();

      sensor_msgs::msg::PointCloud2 point_cloud = point_cloud_;

      pcl::PointCloud<pcl::PointXYZ> cloud;
      pcl::PCLPointCloud2 pcl_pc2;
      pcl_conversions::toPCL(point_cloud ,pcl_pc2);
      pcl::fromPCLPointCloud2(pcl_pc2, cloud);



      RCLCPP_INFO_STREAM(this->get_logger(), "N_points: " << cloud.size()); 



      // Transformation
      pcl::PointCloud<pcl::PointXYZ> cloud_transformed;
      std::vector<int> labels;


      if(_sensor_inclination!=0.){
        geometry_msgs::msg::TransformStamped tf_stamped;

        tf_stamped.transform.translation.x = 0;
        tf_stamped.transform.translation.y = 0;
        tf_stamped.transform.translation.z = 0;

        tf_stamped.transform.rotation.x = 0;
        tf_stamped.transform.rotation.y = std::sin(_sensor_inclination/2);
        tf_stamped.transform.rotation.z = 0;
        tf_stamped.transform.rotation.w = std::cos(_sensor_inclination/2);


        Eigen::Affine3d tf = tf2::transformToEigen(tf_stamped.transform);
        pcl::transformPointCloud(cloud, cloud_transformed, tf);
      }
      else
        cloud_transformed = cloud;




      // Filtering
      pcl::PointCloud<pcl::PointXYZ> cloud_filtered;
      for(pcl::PointXYZ point : cloud_transformed)
      {
        if (point.z > -_sensor_height &&
            point.z < -_sensor_height+_cone_height &&
            (std::abs(point.y)>_vehicle_hiding_width/2 || point.x>_vehicle_hiding_len) &&
            Eigen::Vector2d(point.x, point.y).norm() > _r_min &&
            Eigen::Vector2d(point.x, point.y).norm() < _r_max)
             
          cloud_filtered.push_back(point);
      }

      // Ground removal
      _segmenter.segment(cloud_filtered, &labels);

      pcl::PointCloud<pcl::PointXYZ> obstacle_cloud;
      obstacle_cloud.header = cloud.header;

      for (size_t i = 0; i < cloud_filtered.size(); ++i) 
        if (labels[i] == 0)
          obstacle_cloud.push_back(cloud_filtered[i]);

      RCLCPP_INFO_STREAM(this->get_logger(), "N_points_obstacle: " << obstacle_cloud.size());  


      
      // Clustering
      std::vector<pcl::PointXYZ> obstacle_cloud_vec;      
      for(pcl::PointXYZ point : obstacle_cloud.points) obstacle_cloud_vec.push_back(point);



      // CSV write
      static int csv_counter = 0;
      if (csv_counter % 10 == 0) write_points_to_csv("src/clustering/src/CSVs/obstacles" + std::to_string(csv_counter/10) + ".csv", obstacle_cloud_vec);
      csv_counter++;



      DBSCAN dbScan(_eps, _min_points, _centroide_max_distance, _sensor_height, _max_dist_to_line, _x_filter, obstacle_cloud_vec);
      dbScan.run();
      std::vector<pcl::PointXYZ> centroids_cloud_vec = dbScan.getCentroids();

      pcl::PointCloud<pcl::PointXYZ> centroids_cloud;
      for(pcl::PointXYZ point : centroids_cloud_vec)
        centroids_cloud.push_back(point);
      
      sensor_msgs::msg::PointCloud2 pc2_msg;

      // Retransformation
      pcl::PointCloud<pcl::PointXYZ> centroids_cloud_transformed;

      if(_sensor_inclination!=0.){
        geometry_msgs::msg::TransformStamped tf_stamped;

        tf_stamped.transform.translation.x = 0;
        tf_stamped.transform.translation.y = 0;
        tf_stamped.transform.translation.z = 0;

        tf_stamped.transform.rotation.x = 0;
        tf_stamped.transform.rotation.y = -std::sin(_sensor_inclination/2);
        tf_stamped.transform.rotation.z = 0;
        tf_stamped.transform.rotation.w = std::cos(_sensor_inclination/2);

        Eigen::Affine3d tf = tf2::transformToEigen(tf_stamped.transform);
        pcl::transformPointCloud(centroids_cloud, centroids_cloud_transformed, tf);
      }
      else
        centroids_cloud_transformed = centroids_cloud;


      // Centroid cloud transformed
      pcl::toROSMsg(centroids_cloud_transformed, pc2_msg);
      pc2_msg.header.frame_id = "/centroid_cloud_frame";
      _centroid_cloud_publisher->publish(pc2_msg);
      
      rclcpp::Time end = this->now();
      double computation_time = (end-start).seconds()*1000;
      if(_debug) RCLCPP_INFO_STREAM(this->get_logger(), "Computation time: " << computation_time << " ms");  

      if(_plot)
      {
        // Point cloud
        pcl::toROSMsg(cloud_transformed, pc2_msg);
        pc2_msg.header.frame_id = "/clustering_frame";
        _point_cloud_plot_publisher->publish(pc2_msg);

        // Obstacle cloud
        pcl::toROSMsg(obstacle_cloud, pc2_msg);
        pc2_msg.header.frame_id = "/clustering_frame";
        _obstacle_cloud_plot_publisher->publish(pc2_msg);

        // Centroid cloud
        pcl::toROSMsg(centroids_cloud, pc2_msg);
        pc2_msg.header.frame_id = "/clustering_frame";
        _centroid_cloud_plot_publisher->publish(pc2_msg);
      }

    }


  private:
    double _vehicle_hiding_len;
    double _vehicle_hiding_width;
    double _r_min;
    double _r_max;
    double _cone_height;
    double _sensor_inclination;
    double _sensor_height;
    double _x_filter;
    double _max_dist_to_line;

    double _eps;
    int _min_points;
    double _centroide_max_distance;

    bool _plot;
    bool _debug;

    GroundSegmentationParams _params;
    GroundSegmentation _segmenter;

    std::unique_ptr<tf2_ros::Buffer> _tf_buffer;
    std::shared_ptr<tf2_ros::TransformListener> _tf_listener;

    rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr _point_cloud_subscription;

    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr _point_cloud_plot_publisher;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr _obstacle_cloud_plot_publisher;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr _centroid_cloud_plot_publisher;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr _centroid_cloud_publisher;

    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr _sync_trigger_publisher;
};


int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Clustering>());
  rclcpp::shutdown();
  return 0;
}
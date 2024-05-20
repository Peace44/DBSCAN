cmake_minimum_required(VERSION 3.8)
project(clustering)
set(CMAKE_BUILD_TYPE "Release")

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(pcl_msgs REQUIRED)

find_package(PCL REQUIRED)
find_package(Boost REQUIRED)

include_directories(${PCL_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_executable(clustering src/clustering.cpp src/bin.cpp src/ground_segmentation.cpp src/segment.cpp src/dbscan.cpp)

target_include_directories(clustering
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/clustering>
    $<INSTALL_INTERFACE:include>
    ${rclcpp_INCLUDE_DIRS}
    ${pcl_conversions_INCLUDE_DIRS}
    ${eigen_conversions_INCLUDE_DIRS})


target_link_libraries(clustering ${rclcpp_LIBRARIES} ${PCL_LIBRARIES} ${Boost_LIBRARIES} Eigen3::Eigen)
ament_target_dependencies(clustering
  "geometry_msgs"
  "tf2"
  "tf2_ros"
  "sensor_msgs"
  "tf2_eigen"
  "pcl_msgs")


install(TARGETS
  clustering
  DESTINATION lib/${PROJECT_NAME})

ament_package()

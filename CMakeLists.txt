PROJECT(sonar_wall_hough)
cmake_minimum_required(VERSION 2.6)

pkg_check_modules(BA REQUIRED "base-types")
include_directories(${BA_INCLUDE_DIRS})

SET (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/.orogen/config")
INCLUDE(sonar_wall_houghBase)

# FIND_PACKAGE(KDL)
# FIND_PACKAGE(OCL)


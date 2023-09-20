cmake_minimum_required(VERSION 3.21)

project(UsingDoxygen LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Doxygen REQUIRED)
find_package(Eigen REQUIRED)
find_package(GLEW REQUIRED)

include_directories(${EIGEN_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})

add_library(test test.cpp test.hpp)

doxygen_add_docs(doxygen test.cpp test.hpp ALL)
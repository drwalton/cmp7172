cmake_minimum_required(VERSION 3.21)

project(OpenCVUsage LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(OpenCV REQUIRED)
find_package(Eigen REQUIRED)

include_directories(.)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${Eigen_INCLUDE_DIRS})

include_directories(3rdparty)

set(LIBRARIES
    optimized
        ${OpenCV_LIBRARIES}
    debug
        ${OpenCV_LIBRARIES_DEBUG}
)

add_executable(opencv_usage opencv_usage.cpp)

set_target_properties(opencv_usage PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${OpenCV_DLL_DIR}")

target_link_libraries(opencv_usage ${LIBRARIES})

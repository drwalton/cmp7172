cmake_minimum_required(VERSION 3.21)

project(LinearAlgebra LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Eigen REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(SDL_FontCache REQUIRED)
find_package(OpenCV REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Assimp REQUIRED)

include_directories(.)
include_directories(${Eigen_INCLUDE_DIRS})
include_directories(${SDL_INCLUDE_DIRS})
include_directories(${SDL_TTF_INCLUDE_DIR})
include_directories(${SDL_FontCache_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${Assimp_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})

add_subdirectory(glhelper)

set(LIBRARIES
    ${SDL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${SDL_FontCache_LIBRARY}
    ${SDL_TTF_LIBRARY}
    ${Assimp_LIBRARIES}
    glhelper
    optimized
        ${OpenCV_LIBRARIES}
    debug
        ${OpenCV_LIBRARIES_DEBUG}
)

function(add_executable_rtg name)
    add_executable(${name}
        ${name}.cpp
        ${HEADERS})

    target_link_libraries(${name} ${LIBRARIES})

    target_compile_features(${name} PRIVATE cxx_std_17)

    set_target_properties(${name} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${SDL_DLL_DIR};${SDL_TTF_DLL_DIR};${OpenCV_DLL_DIR};${GLEW_DLL_DIR};${Assimp_DLL_DIR};%PATH%")
endfunction()

add_executable_rtg(tut_00_eigen_basics)
add_executable_rtg(tut_01_multiplication_inverses)
add_executable_rtg(tut_02_dot_cross_products)
add_executable_rtg(ex_00_transformations)
add_executable_rtg(ex_01_transforming_normals)
add_executable_rtg(ex_02_surfel_cloud)
add_executable_rtg(ex_03_mesh_normals)

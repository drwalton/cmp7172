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
    set(SHADER_LIST)
    if(${ARGC} GREATER 1)
        list(SUBLIST ARGV 1 -1 EXTRA_ARGS)
        foreach(ARG ${EXTRA_ARGS})
            list(APPEND SHADER_LIST ${PROJECT_SOURCE_DIR}/shaders/${ARG})
        endforeach()
    endif()


    add_executable(${name}
        ${name}.cpp
        ${HEADERS}
	${SHADER_LIST})

    source_group(Shaders FILES ${SHADER_LIST})

    target_link_libraries(${name} ${LIBRARIES})

    target_compile_features(${name} PRIVATE cxx_std_17)

    set_target_properties(${name} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${SDL_DLL_DIR};${SDL_TTF_DLL_DIR};${OpenCV_DLL_DIR};${GLEW_DLL_DIR};${Assimp_DLL_DIR};%PATH%")
endfunction()

add_executable_rtg(tut_00_eigen_basics)
add_executable_rtg(tut_01_multiplication_inverses)
add_executable_rtg(tut_02_dot_cross_products)
add_executable_rtg(ex_00_transformations Wireframe.vert Wireframe.geom Wireframe.frag)
add_executable_rtg(ex_01_transforming_normals DrawNormalsSetNormMat.vert DrawNormals.geom DrawNormals.frag Gooch.vert Gooch.frag)
add_executable_rtg(ex_02_surfel_cloud DrawNormals.vert DrawNormals.geom DrawNormals.frag Gooch.vert Gooch.frag)
add_executable_rtg(ex_03_mesh_normals DrawNormals.vert DrawNormals.geom DrawNormals.frag Gooch.vert Gooch.frag)

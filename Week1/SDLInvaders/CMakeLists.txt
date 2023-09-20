cmake_minimum_required(VERSION 3.21)

project(SDLInvaders LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(SDL REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(SDL_FontCache REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(.)
include_directories(${SDL_INCLUDE_DIRS})
include_directories(${SDL_TTF_INCLUDE_DIR})
include_directories(${SDL_FontCache_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(3rdparty)

set(LIBRARIES
    ${SDL_LIBRARY}
    ${SDLMAIN_LIBRARY}
    ${SDL_FontCache_LIBRARY}
    ${SDL_TTF_LIBRARY}
    optimized
        ${OpenCV_LIBRARIES}
    debug
        ${OpenCV_LIBRARIES_DEBUG}
)

add_executable(sdl_invaders sdl_invaders.cpp)

set_target_properties(sdl_invaders PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${SDL_DLL_DIR};${SDL_TTF_DLL_DIR};${OpenCV_DLL_DIR}")

target_link_libraries(sdl_invaders ${LIBRARIES})

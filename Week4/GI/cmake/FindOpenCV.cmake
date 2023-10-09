
find_library(OpenCV_LIBRARY
  opencv_world480
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/opencv-4.8.0/build/x64/vc16/lib/
    ${PROJECT_SOURCE_DIR}/../3rdParty/opencv-4.8.0/build/x64/vc16/lib/
    ${PROJECT_SOURCE_DIR}/../../3rdParty/opencv-4.8.0/build/x64/vc16/lib/
)

find_library(OpenCV_LIBRARY_DEBUG
  opencv_world480d
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/opencv-4.8.0/build/x64/vc16/lib/
    ${PROJECT_SOURCE_DIR}/../3rdParty/opencv-4.8.0/build/x64/vc16/lib/
    ${PROJECT_SOURCE_DIR}/../../3rdParty/opencv-4.8.0/build/x64/vc16/lib/
)

set(OpenCV_LIBRARIES ${OpenCV_LIBRARY})
set(OpenCV_LIBRARIES_DEBUG ${OpenCV_LIBRARY_DEBUG})

find_path(OpenCV_INCLUDE_DIR
  opencv2/core.hpp
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/opencv-4.8.0/build/include
    ${PROJECT_SOURCE_DIR}/../3rdParty/opencv-4.8.0/build/include
    ${PROJECT_SOURCE_DIR}/../../3rdParty/opencv-4.8.0/build/include
)

find_path(OpenCV_DLL_DIR
  opencv_world480.dll
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/opencv-4.8.0/build/x64/vc16/bin/
    ${PROJECT_SOURCE_DIR}/../3rdParty/opencv-4.8.0/build/x64/vc16/bin/
    ${PROJECT_SOURCE_DIR}/../../3rdParty/opencv-4.8.0/build/x64/vc16/bin/
)

set(OpenCV_INCLUDE_DIRS ${OpenCV_INCLUDE_DIR})

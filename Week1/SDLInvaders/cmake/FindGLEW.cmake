
find_library(GLEW_LIBRARY
  glew32
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/glew-2.1.0/lib/Release/x64
    ${PROJECT_SOURCE_DIR}/../3rdParty/glew-2.1.0/lib/Release/x64
    ${PROJECT_SOURCE_DIR}/../../3rdParty/glew-2.1.0/lib/Release/x64
    )

set(GLEW_LIBRARIES ${GLEW_LIBRARY})

find_path(GLEW_INCLUDE_DIR
  GL/glew.h
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/glew-2.1.0/include
    ${PROJECT_SOURCE_DIR}/../3rdParty/glew-2.1.0/include
    ${PROJECT_SOURCE_DIR}/../../3rdParty/glew-2.1.0/include)

set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})

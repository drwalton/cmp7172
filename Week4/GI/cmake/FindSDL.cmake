find_library(SDL_LIBRARY
  SDL2
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/SDL2-2.28.2/lib/x64
    ${PROJECT_SOURCE_DIR}/../3rdParty/SDL2-2.28.2/lib/x64
    ${PROJECT_SOURCE_DIR}/../../3rdParty/SDL2-2.28.2/lib/x64
)

find_library(SDL_MAIN_LIBRARY
  SDL2main
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/SDL2-2.28.2/lib/x64
    ${PROJECT_SOURCE_DIR}/../3rdParty/SDL2-2.28.2/lib/x64
    ${PROJECT_SOURCE_DIR}/../../3rdParty/SDL2-2.28.2/lib/x64
)

set(SDL_LIBRARIES ${SDL_LIBRARY} ${SDL_MAIN_LIBRARY})

find_path(SDL_INCLUDE_DIR
  SDL.h
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/SDL2-2.28.2/include
    ${PROJECT_SOURCE_DIR}/../3rdParty/SDL2-2.28.2/include
    ${PROJECT_SOURCE_DIR}/../../3rdParty/SDL2-2.28.2/include
)

set(SDL_INCLUDE_DIRS ${SDL_INCLUDE_DIR})

find_path(SDL_DLL_DIR
  SDL2.dll
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/SDL2-2.28.2/lib/x64
    ${PROJECT_SOURCE_DIR}/../3rdParty/SDL2-2.28.2/lib/x64
    ${PROJECT_SOURCE_DIR}/../../3rdParty/SDL2-2.28.2/lib/x64
)

find_library(SDL_ttf_LIBRARY
  SDL2_ttf
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/SDL2_ttf-2.20.2/lib/x64
    ${PROJECT_SOURCE_DIR}/../3rdParty/SDL2_ttf-2.20.2/lib/x64
    ${PROJECT_SOURCE_DIR}/../../3rdParty/SDL2_ttf-2.20.2/lib/x64
)

find_path(SDL_ttf_INCLUDE_DIR
  SDL_ttf.h
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/SDL2_ttf-2.20.2/include
    ${PROJECT_SOURCE_DIR}/../3rdParty/SDL2_ttf-2.20.2/include
    ${PROJECT_SOURCE_DIR}/../../3rdParty/SDL2_ttf-2.20.2/include
)

find_path(SDL_ttf_DLL_DIR
  SDL2_ttf.dll
  HINTS
    ${PROJECT_SOURCE_DIR}/3rdParty/SDL2_ttf-2.20.2/lib/x64
    ${PROJECT_SOURCE_DIR}/../3rdParty/SDL2_ttf-2.20.2/lib/x64
    ${PROJECT_SOURCE_DIR}/../../3rdParty/SDL2_ttf-2.20.2/lib/x64
)

set(SDL_TTF_INCLUDE_DIR ${SDL_ttf_INCLUDE_DIR})
set(SDL_ttf_INCLUDE_DIRS ${SDL_ttf_INCLUDE_DIR})
set(SDL_TTF_INCLUDE_DIRS ${SDL_ttf_INCLUDE_DIRS})

set(SDL_TTF_LIBRARY ${SDL_ttf_LIBRARY})
set(SDL_ttf_LIBRARIES ${SDL_ttf_LIBRARY})
set(SDL_TTF_LIBRARIES ${SDL_ttf_LIBRARIES})

set(SDL_TTF_DLL_DIR ${SDL_ttf_DLL_DIR})

find_path(SDL_FontCache_INCLUDE_DIRS
        SDL_FontCache.h
	HINTS
		${PROJECT_SOURCE_DIR}/3rdParty/SDL_FontCache
		${PROJECT_SOURCE_DIR}/../3rdParty/SDL_FontCache
		${PROJECT_SOURCE_DIR}/../../3rdParty/SDL_FontCache
)

find_library(SDL_FontCache_LIBRARY
        FontCache.lib
	HINTS
		${PROJECT_SOURCE_DIR}/3rdParty/SDL_FontCache/build/Release
		${PROJECT_SOURCE_DIR}/../3rdParty/SDL_FontCache/build/Release
		${PROJECT_SOURCE_DIR}/../../3rdParty/SDL_FontCache/build/Release
)

set(SDL_FontCache_INCLUDE_DIR ${SDL_FontCache_INCLUDE_DIRS})

set(SDL_FontCache_LIBRARIES ${SDL_FontCache_LIBRARY})

find_path(Assimp_INCLUDE_DIR
        assimp/BaseImporter.h
    HINTS
        ${PROJECT_SOURCE_DIR}/3rdParty/assimp/include
        ${PROJECT_SOURCE_DIR}/../3rdParty/assimp/include
        ${PROJECT_SOURCE_DIR}/../../3rdParty/assimp/include
    )

find_library(Assimp_LIBRARY
        assimp-vc142-mtd
    HINTS
        ${PROJECT_SOURCE_DIR}/3rdParty/assimp/lib/x64
        ${PROJECT_SOURCE_DIR}/../3rdParty/assimp/lib/x64
        ${PROJECT_SOURCE_DIR}/../../3rdParty/assimp/lib/x64
    )

find_path(Assimp_DLL_DIR
        assimp-vc142-mtd.dll
    HINTS
        ${PROJECT_SOURCE_DIR}/3rdParty/assimp/lib/x64
        ${PROJECT_SOURCE_DIR}/../3rdParty/assimp/lib/x64
        ${PROJECT_SOURCE_DIR}/../../3rdParty/assimp/lib/x64
    )

set(Assimp_INCLUDE_DIRS ${Assimp_INCLUDE_DIR})
set(Assimp_LIBRARIES ${Assimp_LIBRARY})

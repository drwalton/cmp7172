find_path(Bullet_INCLUDE_DIR
        bullet/btBulletCollisionCommon.h
    HINTS
        ${PROJECT_SOURCE_DIR}/3rdParty/Bullet3.25/include
        ${PROJECT_SOURCE_DIR}/../3rdParty/Bullet3.25/include
        ${PROJECT_SOURCE_DIR}/../../3rdParty/Bullet3.25/include
)

set(Bullet_LIBRARIES)
set(Bullet_LIBRARIES_DEBUG)

function(find_bullet_library name)
    find_library(Bullet_${name}_LIBRARY
            ${name}
        HINTS
            ${PROJECT_SOURCE_DIR}/3rdParty/Bullet3.25/lib
            ${PROJECT_SOURCE_DIR}/../3rdParty/Bullet3.25/lib
            ${PROJECT_SOURCE_DIR}/../../3rdParty/Bullet3.25/lib
    )
    find_library(Bullet_${name}_LIBRARY_DEBUG
            ${name}_Debug
        HINTS
            ${PROJECT_SOURCE_DIR}/3rdParty/Bullet3.25/lib
            ${PROJECT_SOURCE_DIR}/../3rdParty/Bullet3.25/lib
            ${PROJECT_SOURCE_DIR}/../../3rdParty/Bullet3.25/lib
    )
    list(APPEND Bullet_LIBRARIES ${Bullet_${name}_LIBRARY})
    set(Bullet_LIBRARIES ${Bullet_LIBRARIES} PARENT_SCOPE)

    list(APPEND Bullet_LIBRARIES_DEBUG ${Bullet_${name}_LIBRARY_DEBUG})
    set(Bullet_LIBRARIES_DEBUG ${Bullet_LIBRARIES_DEBUG} PARENT_SCOPE)
endfunction()

find_bullet_library(Bullet2FileLoader)
find_bullet_library(Bullet3Collision)
find_bullet_library(Bullet3Common)
find_bullet_library(Bullet3Dynamics)
find_bullet_library(Bullet3Geometry)
find_bullet_library(Bullet3OpenCL_clew)
find_bullet_library(BulletCollision)
find_bullet_library(BulletDynamics)
find_bullet_library(BulletInverseDynamics)
find_bullet_library(BulletSoftBody)
find_bullet_library(LinearMath)


message(${Bullet_LIBRARIES})

set(Bullet_INCLUDE_DIRS ${Bullet_INCLUDE_DIR})
set(BULLET_INCLUDE_DIR ${Bullet_INCLUDE_DIR})
set(BULLET_INCLUDE_DIRS ${Bullet_INCLUDE_DIR})

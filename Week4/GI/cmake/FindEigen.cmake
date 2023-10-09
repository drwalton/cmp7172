find_path(EIGEN_INCLUDE_DIR
        Eigen/Core
    HINTS
		${PROJECT_SOURCE_DIR}/3rdParty/eigen-3.4.0/
		${PROJECT_SOURCE_DIR}/../3rdParty/eigen-3.4.0/
		${PROJECT_SOURCE_DIR}/../../3rdParty/eigen-3.4.0/
)

set(EIGEN_INCLUDE_DIRS ${EIGEN_INCLUDE_DIR})
set(Eigen_INCLUDE_DIR ${EIGEN_INCLUDE_DIR})
set(Eigen_INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS})

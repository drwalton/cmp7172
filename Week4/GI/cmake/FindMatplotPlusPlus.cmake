find_path(MatplotPlusPlus_INCLUDE_DIR
        matplot/matplot.h
    HINTS
		${PROJECT_SOURCE_DIR}/3rdParty/matplotplusplus/source/
		${PROJECT_SOURCE_DIR}/../3rdParty/matplotplusplus/source/
		${PROJECT_SOURCE_DIR}/../../3rdParty/matplotplusplus/source/
)

find_library(MatplotPlusPlus_LIBRARY
        matplot
    HINTS
		${PROJECT_SOURCE_DIR}/3rdParty/matplotplusplus/build/source/matplot/debug/
		${PROJECT_SOURCE_DIR}/../3rdParty/matplotplusplus/build/source/matplot/debug/
		${PROJECT_SOURCE_DIR}/../../3rdParty/matplotplusplus/build/source/matplot/debug/
)

find_library(Nodesoup_LIBRARY
        nodesoup
    HINTS
		${PROJECT_SOURCE_DIR}/3rdParty/matplotplusplus/build/source/3rdParty/Debug/
		${PROJECT_SOURCE_DIR}/../3rdParty/matplotplusplus/build/source/3rdParty/Debug/
		${PROJECT_SOURCE_DIR}/../../3rdParty/matplotplusplus/build/source/3rdParty/Debug/
)

set(MatplotPlusPlus_INCLUDE_DIRS ${MatplotPlusPlus_INCLUDE_DIR})
set(MatplotPlusPlus_LIBRARIES ${MatplotPlusPlus_LIBRARY} ${Nodesoup_LIBRARY})

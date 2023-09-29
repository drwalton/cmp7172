find_path(glText_INCLUDE_DIR
        gltext.h
	HINTS
		${PROJECT_SOURCE_DIR}/3rdParty/glText
		${PROJECT_SOURCE_DIR}/../3rdParty/glText
		${PROJECT_SOURCE_DIR}/../../3rdParty/glText
)


set(glText_INCLUDE_DIRS ${glText_INCLUDE_DIR})
set(GLTEXT_INCLUDE_DIR ${glText_INCLUDE_DIR})
set(GLTEXT_INCLUDE_DIRS ${glText_INCLUDE_DIRS})

add_library(glhelper
	Entity.cpp
	Exception.cpp
	GLBuffer.cpp
	Mesh.cpp
	Renderable.cpp
	RotateViewer.cpp
	ShaderProgram.cpp

	Constants.hpp
	Entity.hpp
	Exception.hpp
	GLBuffer.hpp
	Mesh.hpp
	Renderable.hpp
	RotateViewer.hpp
	ShaderProgram.hpp
)

target_compile_features(glhelper PRIVATE cxx_std_17)


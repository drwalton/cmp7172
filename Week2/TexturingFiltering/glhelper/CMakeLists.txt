add_library(glhelper
	Entity.cpp
	Exception.cpp
	FlyViewer.cpp
	GLBuffer.cpp
	Matrices.cpp
	Mesh.cpp
	Renderable.cpp
	RotateViewer.cpp
	ShaderProgram.cpp
	Viewer.cpp

	Constants.hpp
	Entity.hpp
	Exception.hpp
	FlyViewer.hpp
	GLBuffer.hpp
	Matrices.hpp
	Mesh.hpp
	Renderable.hpp
	RotateViewer.hpp
	ShaderProgram.hpp
	Viewer.hpp
)

target_compile_features(glhelper PRIVATE cxx_std_17)


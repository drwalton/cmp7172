add_library(glhelper
	CubemapTexture.cpp
	Entity.cpp
	Exception.cpp
	FlyViewer.cpp
	GLBuffer.cpp
	Matrices.cpp
	Mesh.cpp
	Renderable.cpp
	RotateViewer.cpp
	ShaderProgram.cpp
	Texture.cpp
	Viewer.cpp

	Constants.hpp
	CubemapTexture.hpp 
	Entity.hpp
	Exception.hpp
	FlyViewer.hpp
	GLBuffer.hpp
	Matrices.hpp
	Mesh.hpp
	Renderable.hpp
	RotateViewer.hpp
	ShaderProgram.hpp
	Texture.hpp
	Viewer.hpp
)

target_compile_features(glhelper PRIVATE cxx_std_17)


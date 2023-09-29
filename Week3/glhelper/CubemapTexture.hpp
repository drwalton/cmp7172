#pragma once

#include "Texture.hpp"

namespace glhelper {

//!\brief Class encapsulating a (2D) OpenGL texture.
class CubemapTexture
{
public:
	explicit CubemapTexture(
    	GLenum target, GLenum internalFormat, GLsizei width,
    	GLsizei height, GLint border, GLenum format,
    	const GLvoid *data = nullptr);
	~CubemapTexture() throw();
	
	void update(GLenum face, void *data);

	void bindToImageUnit(GLuint unit);
	void unbind();
	
	Texture texture;
};

}


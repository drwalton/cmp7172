#include "CubemapTexture.hpp"

namespace glhelper {

CubemapTexture::CubemapTexture(GLenum target, GLenum internalFormat,
	GLsizei width, GLsizei height, GLint border, GLenum format,
	const GLvoid *data)
	:texture(target, internalFormat, width, height, border, format,
		GL_TEXTURE_CUBE_MAP, data)
{}

CubemapTexture::~CubemapTexture() throw()
{}

void CubemapTexture::update(GLenum face, void *data)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture.tex());
	glTexImage2D(face, 0, texture.internalFormat_,
		texture.width_, texture.height_, 0,
		texture.format_, texture.type_, data);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void CubemapTexture::bindToImageUnit(GLuint unit)
{
	texture.bindToImageUnit(unit);
}

void CubemapTexture::unbind()
{
	texture.unbind();
}

}

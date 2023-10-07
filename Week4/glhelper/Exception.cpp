#include "Exception.hpp"

#include <GL/glew.h>
#include <stdexcept>

void throwOnGlError_(const std::string &info)
{
	GLenum err = glGetError();
	std::string errname = glErrToString(err);
	if (err != GL_NO_ERROR) {
		throw std::runtime_error(info + " " + errname);
	}
}

std::string glErrToString(GLenum err)
{
	switch (err) {
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";
	}
	return "UNKNOWN_GL_ERROR_ENUM";
}


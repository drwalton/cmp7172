#pragma once

#include <string>
#include <GL/glew.h>

std::string glErrToString(GLenum err);

#ifdef _DEBUG
#define throwOnGlError() \
	throwOnGlError_("GL Error in file " __FILE__ ": ");
#else //_DEBUG
#define throwOnGlError() 
#endif //_DEBUG
void throwOnGlError_(const std::string &info);


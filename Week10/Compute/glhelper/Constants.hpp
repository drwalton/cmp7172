#pragma once

#include <GL/glew.h>

namespace glhelper {

enum UniformLocation : GLint {
	VERT = 0,
	NORM = 1,
	TEX = 2,
	COLOR = 3,
	TANGENT = 4,
	BITANGENT = 5,
	BONE_INDICES = 6,
	BONE_WEIGHTS = 7
};

enum UniformBlock : GLuint {
	CAMERA = 0
};


#ifndef M_PI
# define M_PI           3.14159265358979323846  /* pi */
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923132169163975144
#endif

}


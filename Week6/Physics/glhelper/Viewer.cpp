#include "Viewer.hpp"
#include <iostream>

namespace glhelper {

#ifndef M_PI_4
#define M_PI_4 0.78539816339f
#endif

Viewer::Viewer()
:buffer_(&block_, sizeof(CameraBlock))
{
	bindCameraBlock();
}

Viewer::~Viewer() throw()
{}

void Viewer::bindCameraBlock()
{
	buffer_.bindRange(UniformBlock::CAMERA);
}

void Viewer::updateBuffer()
{
	buffer_.update(&block_);
}

}

#pragma once

#include <Eigen/Dense>
#include <SDL.h>
#include "GLBuffer.hpp"

namespace glhelper {

//!\brief A Viewer which rotates about the origin.
class RotateViewer
{
public:
	RotateViewer(int width, int height);
	virtual ~RotateViewer() throw();
	void processEvent(const SDL_Event &e);
	Eigen::Matrix3f rotation() const;
	Eigen::Vector3f position() const;
	void distance(float dist);
	void resize(size_t width, size_t height);
	Eigen::Matrix4f worldToCam() const;
	void bindCameraBlock();
private:
	float theta_, phi_, thetaSpeed_, phiSpeed_;
	float delta_, deltaSpeed_;
	Eigen::Matrix4f translate_, rotate_, perspective_;
	UniformBuffer buffer_;
	CameraBlock block_;
	void updateRotation();
	void updateTranslation();
	void updateBlock();
	void updateBuffer();
};

}


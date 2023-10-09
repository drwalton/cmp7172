#pragma once

#include <Eigen/Dense>
#include <SDL.h>
#include "GLBuffer.hpp"
#include "Viewer.hpp"

namespace glhelper {

//!\brief A Viewer which rotates about the origin.
class RotateViewer : public Viewer
{
public:
	RotateViewer(int width, int height);
	virtual ~RotateViewer() throw();
	void processEvent(const SDL_Event &e);
	virtual void update();
	Eigen::Matrix3f rotation() const;
	Eigen::Vector3f position() const;
	void distance(float dist);
	void resize(size_t width, size_t height);
	Eigen::Matrix4f worldToCam() const;
private:
	float theta_, phi_, thetaSpeed_, phiSpeed_;
	float delta_, deltaSpeed_;
	Eigen::Matrix4f translate_, rotate_, perspective_;
	void updateRotation();
	void updateTranslation();
	void updateBlock();
};

}


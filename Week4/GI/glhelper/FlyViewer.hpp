#pragma once

#include "Viewer.hpp"

namespace glhelper {

//!\brief A Viewer which
class FlyViewer : public Viewer
{
public:
	FlyViewer(int width, int height);
	virtual ~FlyViewer() throw();
	virtual void processEvent(const SDL_Event &e);
	virtual void update();
	
	Eigen::Matrix3f rotation() const;
	Eigen::Vector3f position() const;

	void rotation(float theta, float phi);
	void position(const Eigen::Vector3f &p);

	virtual void resize(size_t width, size_t height);
	virtual Eigen::Matrix4f worldToCam() const;
private:
	float theta_, phi_, thetaSpeed_, phiSpeed_;
	Eigen::Vector3f translation_;
	Eigen::Matrix4f translate_, rotate_, perspective_;
	void updateRotation();
	void updateTranslation();
	void updateBlock();
};

}


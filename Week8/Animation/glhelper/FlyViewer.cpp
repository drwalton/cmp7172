#include "FlyViewer.hpp"
#include "Matrices.hpp"
#include "Exception.hpp"
#include <iostream>

namespace glhelper {

#ifndef M_PI_4
#define M_PI_4 0.78539816339f
#endif

const float zNear = 0.01f;
const float zFar = 50.f;
const float fov = M_PI_4;
const float moveSpeed = 0.1f;

FlyViewer::FlyViewer(int width, int height)
:Viewer(), theta_(0.f), phi_(0.f),
thetaSpeed_(0.005f), phiSpeed_(0.005f),
translation_(Eigen::Vector3f(0.f, 0.f, 0.f)),
rotate_(Eigen::Matrix4f::Identity())
{
	perspective_ = perspective(fov, float(width)/float(height), zNear, zFar);
	updateTranslation();
	updateRotation();
	updateBuffer();
	throwOnGlError();
	bindCameraBlock();
}

FlyViewer::~FlyViewer() throw()
{}

void FlyViewer::processEvent(const SDL_Event &event)
{
	if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_WINDOWEVENT) {
		static int lastx = event.motion.x;
		static int lasty = event.motion.y;
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.motion.state & SDL_BUTTON(1)) {
				lastx = event.motion.x;
				lasty = event.motion.y;
			}
		}
		if (event.type == SDL_MOUSEMOTION) {
			if (event.motion.state & SDL_BUTTON(1)) {
				theta_ += (event.motion.x - lastx) * thetaSpeed_;
				phi_ += (event.motion.y - lasty) * phiSpeed_;
				lastx = event.motion.x;
				lasty = event.motion.y;
				updateRotation();
				updateBuffer();
			}
		}
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
			int width = event.window.data1;
			int height = event.window.data2;
			resize(width, height);
		}
	}
}

void FlyViewer::update()
{
	static const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if(keystate[SDL_SCANCODE_W] || keystate[SDL_SCANCODE_A] ||
		keystate[SDL_SCANCODE_S] || keystate[SDL_SCANCODE_D] ||
		keystate[SDL_SCANCODE_Q] || keystate[SDL_SCANCODE_E]) {
		Eigen::Vector4f translate(0.f, 0.f, 0.f, 1.f);
		float moveAmt = moveSpeed;
		if(keystate[SDL_SCANCODE_LSHIFT]) {
			moveAmt *= 2.f;
		}
		if(keystate[SDL_SCANCODE_W]) {
			translate.z() += moveAmt;
		}
		if(keystate[SDL_SCANCODE_A]) {
			translate.x() += moveAmt;
		}
		if(keystate[SDL_SCANCODE_D]) {
			translate.x() -= moveAmt;
		}
		if(keystate[SDL_SCANCODE_S]) {
			translate.z() -= moveAmt;
		}
		if(keystate[SDL_SCANCODE_Q]) {
			translate.y() -= moveAmt;
		}
		if(keystate[SDL_SCANCODE_E]) {
			translate.y() += moveAmt;
		}
		translate = rotate_.inverse() * translate;
		translation_ += Eigen::Vector3f(translate.x()/translate.w(),
							 translate.y()/translate.w(), translate.z()/translate.w());
		updateTranslation();
		updateBuffer();
	}
}

Eigen::Matrix3f FlyViewer::rotation() const
{
	return rotate_.block<3, 3>(0, 0);
}

Eigen::Vector3f FlyViewer::position() const
{
	Eigen::Vector4f pos = (translate_ * rotate_).inverse() * Eigen::Vector4f(0.f, 0.f, 0.f, 1.f);
	return pos.block<3, 1>(0, 0);
}

void FlyViewer::rotation(float theta, float phi)
{
	theta_ = theta;
	phi_ = phi;
	updateRotation();
	updateBuffer();
}

void FlyViewer::position(const Eigen::Vector3f & p)
{
	translation_ = p;
	updateTranslation();
	updateBuffer();
}

void FlyViewer::resize(size_t width, size_t height)
{
	float aspect = float(width)/float(height);
	perspective_ = perspective(fov, aspect, zNear, zFar);
	updateBlock();
	updateBuffer();
	glViewport(0, 0, width, height);
}

Eigen::Matrix4f FlyViewer::worldToCam() const
{
	return rotate_ * translate_;
}

void FlyViewer::updateRotation()
{
	auto rotate3 =
		Eigen::AngleAxisf(phi_, Eigen::Vector3f(1.f, 0.f, 0.f)).matrix() *
		Eigen::AngleAxisf(theta_, Eigen::Vector3f(0.f, 1.f, 0.f)).matrix();
	rotate_.block<3, 3>(0, 0) = rotate3;
	updateBlock();
}

void FlyViewer::updateTranslation()
{
	translate_ = Eigen::Matrix4f::Identity();
	translate_(0, 3) = translation_.x();
	translate_(1, 3) = translation_.y();
	translate_(2, 3) = translation_.z();
	updateBlock();
}

void FlyViewer::updateBlock()
{
	block_.worldToClip = perspective_ * rotate_ * translate_;
	block_.cameraPos = (rotate_ * translate_).inverse() * Eigen::Vector4f(0.f, 0.f, 0.f, 1.f);
	block_.cameraDir = rotate_.inverse() * Eigen::Vector4f(0.f, 0.f, -1.f, 1.f);
}

}

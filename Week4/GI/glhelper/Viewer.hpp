#pragma once

#include <SDL.h>
#include <Eigen/Dense>
#include "Constants.hpp"
#include "GLBuffer.hpp"

namespace glhelper {

	//!\brief Abstract class encapsulating a 3D viewer, which interprets user input
	//!       (as supplied by SDL) and uses this to move an OpenGL camera.
	class Viewer
	{
	public:
		Viewer();
		virtual ~Viewer() throw();

		virtual void processEvent(const SDL_Event& e) = 0;
		virtual void update() = 0;

		void bindCameraBlock();

		virtual void resize(size_t width, size_t height) = 0;

		virtual Eigen::Matrix4f worldToCam() const = 0;
	protected:
		void updateBuffer();

		UniformBuffer buffer_;
		CameraBlock block_;
	};

}

#pragma once

#include "Entity.hpp"
#include "ShaderProgram.hpp"

namespace glhelper {

//!\brief Abstract class encapsulating an Entity capable of being rendered.
class Renderable : public Entity
{
public:
	explicit Renderable(const Eigen::Matrix4f &modelToWorld = Eigen::Matrix4f::Identity());

	virtual void render() = 0;
	virtual void render(ShaderProgram&) = 0;
};

}


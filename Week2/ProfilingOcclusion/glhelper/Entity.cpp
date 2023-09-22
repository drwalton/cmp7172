#include "Entity.hpp"
#include <Eigen/Dense>

namespace glhelper {

Entity::Entity(const Eigen::Matrix4f &modelToWorld)
:modelToWorld_(modelToWorld)
{}

Entity::~Entity() throw()
{}

void Entity::modelToWorld(const Eigen::Matrix4f &m)
{
	modelToWorld_ = m;
}

Eigen::Matrix4f Entity::modelToWorld() const
{
	return modelToWorld_;
}

Eigen::Matrix3f Entity::normToWorld() const
{
	return modelToWorld_.block<3,3>(0,0).inverse().transpose();
}

}

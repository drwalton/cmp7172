#pragma once

#include <Eigen/Dense>

namespace glhelper {

//!\brief Class representing an object in a 3D world. Has a transform indicating
//!       its location in the world.
class Entity
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	explicit Entity(const Eigen::Matrix4f &modelToWorld = Eigen::Matrix4f::Identity());
	virtual ~Entity() throw();

	void modelToWorld(const Eigen::Matrix4f &m);
	Eigen::Matrix4f modelToWorld() const;

	Eigen::Matrix3f normToWorld() const;
private:
	Eigen::Matrix4f modelToWorld_;
};

}


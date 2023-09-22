#include "Matrices.hpp"

Eigen::Matrix4f perspective(float fov, float aspect, float zNear, float zFar)
{
	float tanHalfFovy = tan(fov * 0.5f);

	Eigen::Matrix4f result = Eigen::Matrix4f::Zero();
	result(0, 0) = 1.0f / (aspect * tanHalfFovy);
	result(1, 1) = 1.0f / tanHalfFovy;
	result(2, 2) = (zFar + zNear) / (zNear - zFar);
	result(3, 2) = -1.0f;
	result(2, 3) = (2.0f * zFar * zNear) / (zNear - zFar);

	return result;
}

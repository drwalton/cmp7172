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

Eigen::Matrix4f makeTranslationMatrix(const Eigen::Vector3f& translate)
{
	Eigen::Matrix4f matrix = Eigen::Matrix4f::Identity();
	matrix.block<3, 1>(0, 3) = translate;
	return matrix;
}

Eigen::Matrix4f makeScaleMatrix(const Eigen::Vector3f& scale)
{
	Eigen::Matrix4f matrix = Eigen::Matrix4f::Identity();
	matrix(0, 0) = scale.x();
	matrix(1, 1) = scale.y();
	matrix(2, 2) = scale.z();
	return matrix;
}

Eigen::Matrix4f makeRotationMatrix(const Eigen::Quaternionf& rotation)
{
	Eigen::Matrix4f result = Eigen::Matrix4f::Identity();
	result.block<3, 3>(0, 0) = rotation.toRotationMatrix();
	return result;
}


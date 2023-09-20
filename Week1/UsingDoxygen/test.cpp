#include "test.hpp"

std::vector<Eigen::Vector3f> makeCuboidVerts(float sizeX, float sizeY, float sizeZ) {
	std::vector<Eigen::Vector3f> cuboidVerts{
		Eigen::Vector3f(-0.5f*sizeX,-0.5f*sizeY,-0.5f*sizeZ),
		Eigen::Vector3f(-0.5f*sizeX,-0.5f*sizeY,0.5f*sizeZ),
		Eigen::Vector3f(0.5f*sizeX,-0.5f*sizeY,0.5f*sizeZ),
		Eigen::Vector3f(0.5f*sizeX,-0.5f*sizeY,-0.5f*sizeZ),

		Eigen::Vector3f(-0.5f*sizeX,0.5f*sizeY,-0.5f*sizeZ),
		Eigen::Vector3f(-0.5f*sizeX,0.5f*sizeY,0.5f*sizeZ),
		Eigen::Vector3f(0.5f*sizeX,0.5f*sizeY,0.5f*sizeZ),
		Eigen::Vector3f(0.5f*sizeX,0.5f*sizeY,-0.5f*sizeZ)
	};
	return cuboidVerts;
}

std::vector<GLuint> makeCuboidIndices() {
	return std::vector<GLuint> {
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		0, 4, 7,
		0, 7, 3,

		2, 6, 5,
		2, 5, 1,

		3, 7, 6,
		3, 6, 2,

		1, 5, 4,
		1, 4, 0
	};
}

int roundToInt(float x)
{
	return static_cast<int>(roundf(x));
}

int fibonacci(int x)
{
	if (x <= 1) 
	{
		return x;
	}
	else 
	{
		return fibonacci(x - 1) + fibonacci(x - 2);
	}
}



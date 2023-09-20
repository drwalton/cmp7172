// We'll mainly use the Dense component of Eigen for this course
// It also has facilities for sparse matrices (big, mostly zeros)
// but for computer graphics we mainly use small (3x3, 4x4) dense matrices
#include <Eigen/Dense>
#include <iostream>
// Get M_PI (and others)
#define _USE_MATH_DEFINES 
#include <math.h>

int main(int argc, char* argv[]) {
	// Eigen's vector and matrix types are templated
	// For vectors, you give the datatype, number of elements
	Eigen::Vector<float, 3> myVector;
	// You can access and set x, y, z using these getters/setters
	myVector.x() = 1.f;
	myVector.y() = 0.f;
	myVector.z() = 0.f;

	// Or use << and ,
	myVector << 1.f, 0.f, 0.f;

	// Eigen has overloaded << operators, so you can use cout to print vectors/matrices.
	std::cout << "myVector:\n" << myVector << std::endl;

	// For matrices, you give the datatype, number of rows and number of columns
	// As you see here you can initialise the matrix using the Identity() static function.
	Eigen::Matrix<float, 3, 3> myMatrix = Eigen::Matrix<float, 3, 3>::Identity();

	std::cout << "myMatrix:\n" << myMatrix << std::endl;

	// For common dimensions (3-vectors, 3x3 matrices) there are typedefs you can use:
	Eigen::Vector3f myOtherVector = Eigen::Vector3f::Ones();
	Eigen::Matrix3f myOtherMatrix = Eigen::Matrix3f::Zero();

	// You can also set elements of matrices or vectors by indexing:
	myOtherVector[0] = 0.f;
	std::cout << "myOtherVector:\n" << myOtherVector << std::endl;

	// For vectors you can use the [] or () operators
	myOtherVector(1) = 1.f;
	std::cout << "myOtherVector:\n" << myOtherVector << std::endl;

	// For matrices you have to use ()
	// This accesses row 0, column 1 of the matrix
	myOtherMatrix(0, 1) = 1.f;
	std::cout << "myOtherMatrix:\n" << myOtherMatrix << std::endl;

	//<< and , also works for matrices:
	myOtherMatrix <<
		1.f, 2.f, 3.f,
		4.f, 5.f, 6.f,
		7.f, 8.f, 9.f;
	std::cout << "myOtherMatrix:\n" << myOtherMatrix << std::endl;

	//A little trick that pops up a lot: setting submatrices
	// We've made a rotation matrix, which is 3x3, but want to convert it to a 4x4 homogeneous matrix
	// so we can use it in GL.
	Eigen::Matrix3f rotate = Eigen::AngleAxisf(M_PI, Eigen::Vector3f(1.f, 1.f, 0.f).normalized()).matrix();
	Eigen::Matrix4f rotate4x4 = Eigen::Matrix4f::Identity();

	// We can do this in one line using .block<>():
	rotate4x4.block<3, 3>(0,0) = rotate;

	// As you can see .block returns a submatrix (in this case of size 3x3 starting from (0,0)
	std::cout << "rotate4x4:\n" << rotate4x4 << std::endl;

	return 0;
}

// We'll mainly use the Dense component of Eigen for this course
// It also has facilities for sparse matrices (big, mostly zeros)
// but for computer graphics we mainly use small (3x3, 4x4) dense matrices
#include <Eigen/Dense>
#include <iostream>

int main(int argc, char* argv[]) {
	// Addition and subtraction in Eigen work element-wise, as you'd expect
	Eigen::Vector3f vecOne, vecTwo;
	vecOne << 1.f, 2.f, 3.f;
	vecTwo << 2.f, 3.f, 4.f;
	std::cout << "vecOne + vecTwo\n" << vecOne + vecTwo << std::endl;

	// The * operator in Eigen means matrix multiplication.
	// As a reminder you can multiply two matrices if the inner dimensions match, e.g.
	// 2x3 * 3x4 is fine, because the inner dimensions of both are 3
	// 2x3 * 4x3 won't work, because 3 != 4
	Eigen::Matrix<float, 2, 3> matrixOne = Eigen::Matrix<float, 2, 3>::Ones();
	Eigen::Matrix<float, 3, 4> matrixTwo = Eigen::Matrix<float, 3, 4>::Ones();

	// Notice the dimensions of the output matrix are 2x4 (so 2x3 * 3x4 => 2x4) 
	// i.e. the outer dimensions are kept.
	std::cout << "matrixOne * matrixTwo\n" << matrixOne * matrixTwo << std::endl;

	// Uncomment the below if you want - it won't compile!
	// There's a static_assert in Eigen that checks for this, that will
	// notice this is an INVALID_MATRIX_PRODUCT (the dimensions don't match).
	/*
	Eigen::Matrix<float, 2, 3> matrixThree = Eigen::Matrix<float, 2, 3>::Ones();
	Eigen::Matrix<float, 4, 3> matrixFour = Eigen::Matrix<float, 4, 3>::Ones();
	std::cout << "matrixThree * matrixFour\n" << matrixThree * matrixFour << std::endl;
	*/

	// Similarly you can't matrix multiply two vectors (e.g. 3x1 * 3x1 doesn't work)
	//std::cout << "vecOne * vecTwo\n" << vecOne * vecTwo << std::endl;

	// There are really two ways to matrix multiply vectors
	// Doing 1x3 * 3x1 gives a single number (1x1) as output
	Eigen::Matrix<float, 1, 3> vecThree; vecThree << 1.f, 0.f, 1.f;
	Eigen::Matrix<float, 3, 1> vecFour; vecFour << 0.f, 1.f, 0.f;

	std::cout << "vecThree * vecFour\n" << vecThree * vecFour << std::endl;

	// You might have noticed this is equivalent to the dot product (AKA inner product).
	// Also in this case the dot product is zero as the two vectors are orthogonal.

	// You can also perform a dot product using the .dot() method.
	// For this you don't have to match up the dimensions, so we can
	// dot two Eigen::Vector3s.
	std::cout << "vecOne . vecTwo\n" << vecOne.dot(vecTwo) << std::endl;
	std::cout << "vecThree . vecFour\n" << vecThree.dot(vecFour) << std::endl;

	// The only restriction is the vectors must have the same length:
	// The below code takes a dot of a 3-vector and 4-vector, so won't compile.
	/*
	Eigen::Vector3f vecFive;
	Eigen::Vector4f vecSix;
	std::cout << "vecFive . vecSix\n" << vecFive.dot(vecSix) << std::endl;
	*/
	
	// Eigen can also compute cross products for you using the .cross() method.
	std::cout << "vecOne x vecTwo\n" << vecOne.cross(vecTwo) << std::endl;


	// All real numbers (except 0) have multiplicative inverses
	// For example, 3 * 1/3 = 1
	// The equivalent for a matrix M is another matrix M' so that
	// M * M' is the identity matrix.
	// Only square matrices have multiplicative inverses (other matrices
	// change size when multiplied, so the concept doesn't really work).
	// Eigen has the .inverse() method to find the inverse of a matrix

	Eigen::Matrix3f matrixThree;
	matrixThree <<
		1.f, 0.f, 0.f,
		0.f, 2.f, 0.f,
		0.f, 0.f, 3.f;

	// Here we find the inverse
	std::cout << "matrixThree.inverse()\n" << matrixThree.inverse() << std::endl;

	// And the multiple of the matrix and its inverse is the identity
	std::cout << "matrixThree * matrixThree.inverse()\n" << matrixThree * matrixThree.inverse() << std::endl;
	// Do remember though that due to floating-point errors you won't always get precisely the identity here!

	// Just like the real numbers where 0 has no inverse, some square matrices
	// don't have multiplicative inverses.
	// A matrix only has a multiplicative inverse if its rows/columns are linearly independent
	// That is, you can't make any of the rows/columns by adding and summing the others

	matrixThree <<
		1.f, 0.f, 0.f,
		0.f, 2.f, 0.f,
		1.f, 2.f, 0.f;

	// Now matrixThree isn't linearly independent anymore, because the third row is the sum of 
	// the first two. If we try to find its inverse we'll get a result with NaN/inf values:
	std::cout << "matrixThree.inverse()\n" << matrixThree.inverse() << std::endl;

	// To check for NaN or inf you may want to use 
	if (!std::isfinite(matrixThree.inverse().maxCoeff())) {
		std::cout << "The inverse of matrixThree contains a NaN or inf value" << std::endl;
	}
	// The maxCoeff normally finds the maximum coefficient in the matrix,
	// but if it contains a NaN this will be returned 
	// Also of course if there's an inf this will be the maximum coefficient.
	// std::isfinite() will be false for both inf and NaN
	// For just NaN you can use std::isnan().

	// If you wanted to be safe you could also check the determinant
	// the determinant of a singular matrix is always 0
	std::cout << "matrixThree.determinant(): " << matrixThree.determinant() << std::endl;

	// Some other tricks - you can multiply matrices or vectors by scalars (floats)
	std::cout << "matrixThree * 2.f\n" << matrixThree * 2.f << std::endl;

	// If you want to do element-wise operations you can convert the Matrix
	// to an Array temporarily
	std::cout << "matrixThree.array() + 2.f\n" << matrixThree.array() + 2.f << std::endl;

	// And convert back to a matrix after to do matrix multiplication etc.
	std::cout << "(matrixThree.array() + 2.f).matrix().determinant()\n" << 
		(matrixThree.array() + 2.f).matrix().determinant() << std::endl;


	return 0;
}


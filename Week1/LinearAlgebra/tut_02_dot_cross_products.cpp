#include <Eigen/Dense>
#include <iostream>
#include <algorithm>
#define NOMINMAX
#include <windows.h>
#define _USE_MATH_DEFINES
#include <math.h>

/*
* Demo showing some properties of dot and cross products.
*/

int main(int argc, char* argv[]) {

	// Given any two vectors (that don't lie along the same line)
	// Their cross product is another vector perpendicular to both.
	// You can work out which direction the cross product points using
	// the right hand rule.
	// Unlike regular multiplication, order matters - in fact
	// vectorOne x vectorTwo = -(vectorTwo x vectorOne)
	// Try swapping the order and the cross product will reverse direction.

	Eigen::Vector3f vectorOne(2.f, 1.f, 0.f), vectorTwo(1.f, 2.f, 1.f);
	vectorOne.normalize(); vectorTwo.normalize();
	
	auto crossProduct = vectorOne.cross(vectorTwo);
	std::cout << "vectorOne x vectorTwo = " << crossProduct << std::endl;

	// We can check it's perpendicular to both by taking dot products.
	// The dot product of two perpendicular vectors is zero.

	std::cout << "vectorOne . crossProduct = " << vectorOne.dot(crossProduct) << std::endl;
	std::cout << "vectorTwo . crossProduct = " << vectorTwo.dot(crossProduct) << std::endl;
	
	// Cross products are important for finding normals to surfaces, which
	// in turn are important for lighting calculations.
	// In this case you'll typically normalise the cross product afterwards

	crossProduct = vectorOne.cross(vectorTwo).normalized();

	// I'd strongly recommend checking out the immersivemath.com courses on linear algebra,
	// particularly the interactive cross product visualiser:
	// http://immersivemath.com/ila/ch04_vectorproduct/ch04.html#fig_vp_interactive_vector_product
	// to get to grips with how the cross product behaves.

	return 0;
}

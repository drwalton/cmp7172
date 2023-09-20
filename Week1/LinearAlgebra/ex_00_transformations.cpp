#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <iostream>
#include <exception>
#include <cmath>
#include "glhelper/ShaderProgram.hpp"
#include "glhelper/RotateViewer.hpp"
#include "glhelper/Mesh.hpp"

/*
* Exercise generating and applying some transformation matrices to a 3D cube.
* Running this code will show two wireframe cubes, in blue and red.
* Change the transform applied to the red cube at line 102 to apply the specified 
* transformations to the red cube's vertices.
*/

const int winWidth = 1024, winHeight = 720;

const Uint64 desiredFrametime = 33;

int main()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
	// Turns on 4x MSAA
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// Prepare window
	SDL_Window* window = SDL_CreateWindow("A test window", 50, 50, winWidth, winHeight, SDL_WINDOW_OPENGL);

	SDL_GLContext context = SDL_GL_CreateContext(window);

	GLenum result = glewInit();
	if (result != GLEW_OK) {
		throw std::runtime_error("GLEW couldn't initialize.");
	}
	glEnable(GL_MULTISAMPLE);

	{
		glhelper::ShaderProgram wireframeShader({ "../shaders/Wireframe.vert", "../shaders/Wireframe.geom", "../shaders/Wireframe.frag" });
		glProgramUniform4f(wireframeShader.get(), wireframeShader.uniformLoc("faceColor"), 0.f, 0.f, 0.f, 0.f);
		glhelper::RotateViewer viewer(winWidth, winHeight);
		glhelper::Mesh cubeMesh, transformedCubeMesh;

		Eigen::Matrix4f modelToWorld = Eigen::Matrix4f::Identity();
		modelToWorld(0, 3) = -3.f;
		cubeMesh.modelToWorld(modelToWorld);
		modelToWorld(0, 3) = 3.f;
		transformedCubeMesh.modelToWorld(modelToWorld);


		std::vector<Eigen::Vector3f> cubeVerts{
			Eigen::Vector3f(-0.5f,-0.5f,-0.5f),
			Eigen::Vector3f(-0.5f,-0.5f,0.5f),
			Eigen::Vector3f(0.5f,-0.5f,0.5f),
			Eigen::Vector3f(0.5f,-0.5f,-0.5f),

			Eigen::Vector3f(-0.5f,0.5f,-0.5f),
			Eigen::Vector3f(-0.5f,0.5f,0.5f),
			Eigen::Vector3f(0.5f,0.5f,0.5f),
			Eigen::Vector3f(0.5f,0.5f,-0.5f)
		};

		std::vector<GLuint> cubeElems{
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

		cubeMesh.vert(cubeVerts);
		cubeMesh.elems(cubeElems);
		cubeMesh.shaderProgram(&wireframeShader);

		// Placeholder: the identity matrix
		Eigen::Matrix3f transformationMatrix;
		transformationMatrix <<
			1.f, 0.f, 2.f,
			0.f, 1.f, 3.f,
			0.f, 0.f, 0.f;

		// *** Your Code Here ***
		// Implement the following transformations:
		// 1. Rotate 45 degrees around the x axis (try using Eigen::AngleAxisf)
		// 2. Euler Angles: write a function that takes 3 angles r_x, r_y and r_z and rotates around
		// the x, y and z axes in that order. Test it with a range of inputs.
		// 3. Create a shear matrix
		// 4. Mirror the cube in the x axis.
		// 5. Scale the cube along the x and y axes.
		// 6. Change the matrix to be a singular matrix of rank 2. How has the cube been affected?
		// 7. Change the matrix to be a singular matrix of rank 1. What has happened to the cube now?

		std::cout << transformationMatrix << std::endl;

		std::vector<Eigen::Vector3f> transformedCubeVerts;
		std::transform(cubeVerts.begin(), cubeVerts.end(), 
			std::back_inserter(transformedCubeVerts), 
			[&transformationMatrix](Eigen::Vector3f in) 
			{return transformationMatrix * in; });

		transformedCubeMesh.vert(transformedCubeVerts);
		transformedCubeMesh.elems(cubeElems);
		transformedCubeMesh.shaderProgram(&wireframeShader);

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		while (!shouldQuit) {
			Uint64 frameStartTime = SDL_GetTicks64();

			while (SDL_PollEvent(&event)) {
				// Check for X of window being clicked, or ALT+F4
				if (event.type == SDL_QUIT) {
					shouldQuit = true;
				}
				else {
					viewer.processEvent(event);
				}
			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glProgramUniform4f(wireframeShader.get(), wireframeShader.uniformLoc("lineColor"), 0.f, 0.f, 1.f, 1.f);
			cubeMesh.render();
			glProgramUniform4f(wireframeShader.get(), wireframeShader.uniformLoc("lineColor"), 1.f, 0.f, 0.f, 1.f);
			transformedCubeMesh.render();
			glDisable(GL_CULL_FACE);

			SDL_GL_SwapWindow(window);

			Uint64 elapsedFrameTime = SDL_GetTicks64() - frameStartTime;
			if (elapsedFrameTime < desiredFrametime) {
				SDL_Delay(desiredFrametime - elapsedFrameTime);
			}
		}
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}


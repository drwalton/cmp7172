#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <iostream>
#include <exception>
#include <cmath>
#include "glhelper/ShaderProgram.hpp"
#include "glhelper/RotateViewer.hpp"
#include "glhelper/FlyViewer.hpp"
#include "glhelper/Mesh.hpp"
#include <opencv2/opencv.hpp>
#define GLT_IMPLEMENTATION
#include <gltext.h>

/* This existing application shows a plane textured with a repeating
* checkerboard, where the texture is displayed with nearest-neighbour filtering.
* Basic controls are enabled (WASD, click and drag to look around, SHIFT go faster, Q&E up & down)
*
* Your task is to set up a range of filtering modes so the user can switch between them, as set out
* in the if/else code at line 119.
* 
* Once you have set up the different modes, test them whilst moving around the scene. What are the
* major differences you can see between the different modes?
* 
*/

const int winWidth = 1024, winHeight = 768;

const Uint64 desiredFrametime = 33;

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	std::cout << message << std::endl;
}


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

	// Prepare window
	SDL_Window* window = SDL_CreateWindow("A test window", 50, 50, winWidth, winHeight, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);


	GLenum result = glewInit();
	if (result != GLEW_OK) {
		throw std::runtime_error("GLEW couldn't initialize.");
	}
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, nullptr);

	gltInit();
	GLTtext* text = gltCreateText();
	gltSetText(text, "NEAREST");

	{
		glhelper::ShaderProgram texturedMeshShader({ "../shaders/TexturedMesh.vert", "../shaders/TexturedMesh.frag" });
		//glhelper::RotateViewer viewer(winWidth, winHeight);
		glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh bigPlaneMesh;

		float bigPlaneSize = 100.f, bigPlaneHeight = -3.f;

		std::vector<Eigen::Vector3f> bigPlanePoints{
			Eigen::Vector3f(-bigPlaneSize * 0.5f, bigPlaneHeight, -bigPlaneSize * 0.5f),
			Eigen::Vector3f(bigPlaneSize * 0.5f, bigPlaneHeight, -bigPlaneSize * 0.5f),
			Eigen::Vector3f(-bigPlaneSize * 0.5f, bigPlaneHeight, bigPlaneSize * 0.5f),
			Eigen::Vector3f(bigPlaneSize * 0.5f, bigPlaneHeight, bigPlaneSize * 0.5f),
		};
		std::vector<Eigen::Vector2f> bigPlaneTexCoords{
			Eigen::Vector2f(0.f, 0.f),
			Eigen::Vector2f(bigPlaneSize, 0.f),
			Eigen::Vector2f(0.f, bigPlaneSize),
			Eigen::Vector2f(bigPlaneSize, bigPlaneSize),
		};
		std::vector<GLuint> bigPlaneElems{ 0,1,2,1,2,3 };

		bigPlaneMesh.vert(bigPlanePoints);
		bigPlaneMesh.tex(bigPlaneTexCoords);
		bigPlaneMesh.elems(bigPlaneElems);

		bigPlaneMesh.shaderProgram(&texturedMeshShader);

		glProgramUniform1i(texturedMeshShader.get(), texturedMeshShader.uniformLoc("tex"), 0);

		cv::Mat image = cv::imread("../images/checkerboard.png");
		cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);

		glGenerateTextureMipmap(texture);
		
		glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, 5000);
		glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// This isn't strictly necessary as GL_REPEAT is the default mode.
		glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		while (!shouldQuit) {
			Uint64 frameStartTime = SDL_GetTicks64();
			viewer.update();

			while (SDL_PollEvent(&event)) {
				// Check for X of window being clicked, or ALT+F4
				if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.sym == SDLK_1) {
						gltSetText(text, "Nearest-neighbour");
						// Perform nearest-neighbour filtering here.
					}
					else if (event.key.keysym.sym == SDLK_2) {
						gltSetText(text, "Linear, no MIP map");
						// Perform linear filtering here (don't use MIP map).
					}
					else if (event.key.keysym.sym == SDLK_3) {
						gltSetText(text, "Linear, MIP map levels nearest-neighbour");
						// Perform linear filtering here and use the MIP map filtering between levels
						// with a nearest-neighbour filter.
					}
					else if (event.key.keysym.sym == SDLK_4) {
						gltSetText(text, "Linear, MIP map levels linear");
						// Perform linear filtering here and use the MIP map filtering between levels
						// with a linear filter.
					}
					else if (event.key.keysym.sym == SDLK_5) {
						// Perform linear filtering here and use the MIP map filtering between levels
						// with a linear filter. Also apply up to 4-sample anisotropic filtering.
					}
				}
				if (event.type == SDL_QUIT) {
					shouldQuit = true;
				}
				viewer.processEvent(event);
			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDisable(GL_CULL_FACE);
			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, texture);

			bigPlaneMesh.render();

			gltBeginDraw();
			gltColor(1.f, 1.f, 1.f, 1.f);
			gltDrawText2D(text, 10.f, 10.f, 1.f);
			gltEndDraw();

			SDL_GL_SwapWindow(window);

			Uint64 elapsedFrameTime = SDL_GetTicks64() - frameStartTime;
			if (elapsedFrameTime < desiredFrametime) {
				SDL_Delay(desiredFrametime - elapsedFrameTime);
			}
		}
	}

	gltDeleteText(text);
	gltTerminate();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

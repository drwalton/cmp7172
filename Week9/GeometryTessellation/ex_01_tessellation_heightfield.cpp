#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <iostream>
#include <exception>
#include <cmath>
#include <random>
#include "glhelper/ShaderProgram.hpp"
#include "glhelper/RotateViewer.hpp"
#include "glhelper/FlyViewer.hpp"
#include "glhelper/Mesh.hpp"
#include "glhelper/Texture.hpp"
#include "glhelper/Matrices.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"

#include <opencv2/opencv.hpp>
#define GLT_IMPLEMENTATION
#include <gltext.h>
/* In this exercise you'll use tessellation shaders to create a detailed heightfield mesh from a low-resolution 2x2 grid of quads.
* This will be an alternative way to add height detail to the stony path image we saw in the parallax mapping exercise.
* This time rather than using sampling trickery to give the illusion of detail, we're actually adding proper geometry!
* 
* Tasks:
* 1. Implement your Tessellation Control shader. This sets tessellation levels for each quad to be subdivided. In more
* advanced shaders you might compute this based on viewing distance - here you can just set all tessellation levels to the value
* of the tessLevel uniform. Remember you're dealing with quads, which have 4 outer and 2 inner tessellation levels. You only need
* to set tessellation levels for gl_InvocationID 0. Don't forget to pass through the position and texture coordinates!
* 2. Implement your Tessellation Evaluation shader. For this you need to find the position and texture coordinate for each
* tessellated vertex, using gl_TessCoord, the quad's input positions and texture coordinates and the depth texture.
* To do this you need to linearly interpolate the texture coordinates and positions (to do this in 2D, you can interpolate 
* in 1D along the top & bottom of the quad, then interpolate between these values from bottom to top.
*/

const int winWidth = 1280, winHeight = 720;

const Uint64 desiredFrametime = 33;

const int meshWidth = 2;

float tessLevel = 2.f;
float depthScaling = 0.2f;

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
	SDL_Window* window = SDL_CreateWindow("Tesselation Shader-Based Heightfield Rendering Exercise", 50, 50, winWidth, winHeight, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);

	GLenum result = glewInit();
	if (result != GLEW_OK) {
		throw std::runtime_error("GLEW couldn't initialize.");
	}

	gltInit();
	GLTtext* text = gltCreateText();
	gltSetText(text, (std::string("Animation demo")).c_str());
	glEnable(GL_MULTISAMPLE);

	{
		glhelper::ShaderProgram heightfieldShader({ 
			"../shaders/Heightfield.vert", "../shaders/Heightfield.tesc", 
			"../shaders/Heightfield.tese", "../shaders/Heightfield.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);

		glPatchParameteri(GL_PATCH_VERTICES, 4);

		// Makes an NxN mesh of quads. Change meshWidth to increase or decrease
		// the initial resolution of the mesh, before tessellation.
		glhelper::Mesh heightfieldControlMesh;
		{
			std::vector<Eigen::Vector3f> controlVertices;
			std::vector<Eigen::Vector2f> controlTexCoords;
			for (int y = 0; y < meshWidth-1; ++y) {
				for (int x = 0; x < meshWidth-1; ++x) {
					controlVertices.push_back(Eigen::Vector3f(
						2.f * (float)x / (float)(meshWidth - 1) - 1.0f,
						0.f,
						2.f * (float)y / (float)(meshWidth - 1) - 1.0f));
					controlVertices.push_back(Eigen::Vector3f(
						2.f * (float)(x+1) / (float)(meshWidth - 1) - 1.0f,
						0.f,
						2.f * (float)y / (float)(meshWidth - 1) - 1.0f));
					controlVertices.push_back(Eigen::Vector3f(
						2.f * (float)(x+1) / (float)(meshWidth - 1) - 1.0f,
						0.f,
						2.f * (float)(y+1) / (float)(meshWidth - 1) - 1.0f));
					controlVertices.push_back(Eigen::Vector3f(
						2.f * (float)x / (float)(meshWidth - 1) - 1.0f,
						0.f,
						2.f * (float)(y+1) / (float)(meshWidth - 1) - 1.0f));

					controlTexCoords.push_back(Eigen::Vector2f(
						(float)x / (float)(meshWidth - 1),
						(float)y / (float)(meshWidth - 1)));
					controlTexCoords.push_back(Eigen::Vector2f(
						(float)(x+1) / (float)(meshWidth - 1),
						(float)y / (float)(meshWidth - 1)));
					controlTexCoords.push_back(Eigen::Vector2f(
						(float)(x+1) / (float)(meshWidth - 1),
						(float)(y+1) / (float)(meshWidth - 1)));
					controlTexCoords.push_back(Eigen::Vector2f(
						(float)x / (float)(meshWidth - 1),
						(float)(y+1) / (float)(meshWidth - 1)));
				}
			}
			heightfieldControlMesh.vert(controlVertices);
			heightfieldControlMesh.tex(controlTexCoords);
		}
		heightfieldControlMesh.shaderProgram(&heightfieldShader);
		heightfieldControlMesh.drawMode(GL_PATCHES);

		glProgramUniform1i(heightfieldShader.get(), heightfieldShader.uniformLoc("depthTexture"), 0);
		glProgramUniform1i(heightfieldShader.get(), heightfieldShader.uniformLoc("colorTexture"), 1);
		glProgramUniform1i(heightfieldShader.get(), heightfieldShader.uniformLoc("normalTexture"), 2);

		glProgramUniform1f(heightfieldShader.get(), heightfieldShader.uniformLoc("depthScaling"), depthScaling);
		glProgramUniform1f(heightfieldShader.get(), heightfieldShader.uniformLoc("tessLevel"), tessLevel);
		gltSetText(text, (std::string("Tess level ") + std::to_string(tessLevel) + std::string(" Height scale ") + std::to_string(depthScaling)).c_str());
		

		GLuint colorTexture;
		GLuint depthTexture;
		GLuint normalTexture;
		{
			cv::Mat depthImage = cv::imread("../images/cobblestone_depth.png");
			cv::cvtColor(depthImage, depthImage, cv::COLOR_BGR2GRAY);
			glGenTextures(1, &depthTexture);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, depthImage.cols, depthImage.rows, 0, GL_RED, GL_UNSIGNED_BYTE, depthImage.data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);

			cv::Mat colorImage = cv::imread("../images/cobblestone_albedo.png");
			glGenTextures(1, &colorTexture);
			glBindTexture(GL_TEXTURE_2D, colorTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, colorImage.cols, colorImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, colorImage.data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);

			cv::Mat normalImage = cv::imread("../images/cobblestone_normal.png");
			glGenTextures(1, &normalTexture);
			glBindTexture(GL_TEXTURE_2D, normalTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, colorImage.cols, colorImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, colorImage.data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);

		while (!shouldQuit) {
			Uint64 frameStartTime = SDL_GetTicks64();

			viewer.update();

			while (SDL_PollEvent(&event)) {
				// Check for X of window being clicked, or ALT+F4
				if (event.type == SDL_QUIT) {
					shouldQuit = true;
				}
				else {
					viewer.processEvent(event);
				}

				if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_UP) {
						if (event.key.keysym.sym == SDLK_DOWN) {
							if (event.key.keysym.mod & KMOD_LSHIFT) {
								depthScaling -= 0.01f;
								if (depthScaling < 0.f) depthScaling = 0.f;
							}
							else {
								tessLevel -= 1;
								tessLevel = std::max(tessLevel, 1.f);
							}
						}
						if (event.key.keysym.sym == SDLK_UP) {
							if (event.key.keysym.mod & KMOD_LSHIFT) {
								depthScaling += 0.01f;
							}
							else {
								tessLevel += 1;
							}
						}
						glProgramUniform1f(heightfieldShader.get(), heightfieldShader.uniformLoc("depthScaling"), depthScaling);
						glProgramUniform1f(heightfieldShader.get(), heightfieldShader.uniformLoc("tessLevel"), tessLevel);
					}
					gltSetText(text, (std::string("Tess level ") + std::to_string(tessLevel) + std::string(" Depth scale ") + std::to_string(depthScaling)).c_str());
				}

			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, colorTexture);
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, normalTexture);
			heightfieldControlMesh.render();
			glBindTexture(GL_TEXTURE_2D, 0);

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

		glDeleteTextures(1, &colorTexture);
		glDeleteTextures(1, &depthTexture);
		glDeleteTextures(1, &normalTexture);
	}

	gltDeleteText(text);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

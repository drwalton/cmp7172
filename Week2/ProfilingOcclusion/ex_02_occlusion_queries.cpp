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
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include <opencv2/opencv.hpp>
#define GLT_IMPLEMENTATION
#include <gltext.h>

/* 
* In this exercise you'll perform occlusion queries and display the results.
* The scene contains a large and a small sphere, and you can navigate around the space
* with the WASD keys and the mouse.
* You should carry out occlusion queries to determine whether the small sphere is visible
* or occluded by the larger sphere, and also to count the number of samples generated
* when rendering the smaller sphere.
* The user should be able to switch between running sample count or visibility queries by
* pressing space (note that in the current code this changes the value of the `mode` variable.
* 
* Hints
* -----
* 
* I don't think it's possible to run visibility and sample count queries at the same time
* (at least it hasn't worked for me!) so make sure to only run one or the other depending 
* on the current value of the `mode` variable.
*/

const int winWidth = 1024, winHeight = 768;

const Uint64 desiredFrametime = 33;

enum Mode {
	SHOW_COUNT, SHOW_VISIBILITY
};

Mode mode = Mode::SHOW_VISIBILITY;

void loadMesh(glhelper::Mesh* mesh, const std::string& filename)
{
	Assimp::Importer importer;
	importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	const aiScene* aiscene = importer.GetScene();
	const aiMesh* aimesh = aiscene->mMeshes[0];

	std::vector<Eigen::Vector3f> verts(aimesh->mNumVertices);
	std::vector<Eigen::Vector3f> norms(aimesh->mNumVertices);
	std::vector<Eigen::Vector2f> uvs(aimesh->mNumVertices);
	std::vector<GLuint> elems(aimesh->mNumFaces * 3);
	memcpy(verts.data(), aimesh->mVertices, aimesh->mNumVertices * sizeof(aiVector3D));
	memcpy(norms.data(), aimesh->mNormals, aimesh->mNumVertices * sizeof(aiVector3D));
	for (size_t v = 0; v < aimesh->mNumVertices; ++v) {
		uvs[v][0] = aimesh->mTextureCoords[0][v].x;
		uvs[v][1] = 1.f - aimesh->mTextureCoords[0][v].y;
	}
	for (size_t f = 0; f < aimesh->mNumFaces; ++f) {
		for (size_t i = 0; i < 3; ++i) {
			elems[f * 3 + i] = aimesh->mFaces[f].mIndices[i];
		}
	}

	mesh->vert(verts);
	mesh->norm(norms);
	mesh->elems(elems);
	mesh->tex(uvs);
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
	gltInit();
	GLTtext* text = gltCreateText();
	gltSetText(text, "REPLACE ME WITH PERFORMANCE TIMINGS");

	{
		glhelper::ShaderProgram fixedColorShader({ "../shaders/FixedColor.vert", "../shaders/FixedColor.frag" });
		glProgramUniform4f(fixedColorShader.get(), fixedColorShader.uniformLoc("color"), 0.1f, 0.8f, 0.8f, 1.0f);
		//glhelper::RotateViewer viewer(winWidth, winHeight);
		glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh sphereMesh;

        loadMesh(&sphereMesh, "../models/sphere.obj");
		sphereMesh.shaderProgram(&fixedColorShader);

		Eigen::Matrix4f sphereMatrix1, sphereMatrix2;
		sphereMatrix1 <<
			3.f, 0.f, 0.f, 3.f,
			0.f, 3.f, 0.f, 0.f,
			0.f, 0.f, 3.f, 0.f,
			0.f, 0.f, 0.f, 1.f;
		sphereMatrix2 <<
			1.f, 0.f, 0.f, -3.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f;
		Eigen::Vector4f sphereColor1 = Eigen::Vector4f(0.1f, 0.8f, 0.8f, 1.f);
		Eigen::Vector4f sphereColor2 = Eigen::Vector4f(0.5f, 0.1f, 0.3f, 1.f);


		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// --- Your Code Here ---
		// Generate your query objects (for sample count, and for visibility)

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
				if (event.key.keysym.sym == SDLK_SPACE) {
					if (mode == Mode::SHOW_COUNT) mode = Mode::SHOW_VISIBILITY;
					else mode = Mode::SHOW_COUNT;
				}
			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDisable(GL_CULL_FACE);

			sphereMesh.modelToWorld(sphereMatrix1);
			glProgramUniform4f(fixedColorShader.get(), fixedColorShader.uniformLoc("color"), sphereColor1.x(), sphereColor1.y(), sphereColor1.z(), 1.f);
			sphereMesh.render();

			sphereMesh.modelToWorld(sphereMatrix2);
			glProgramUniform4f(fixedColorShader.get(), fixedColorShader.uniformLoc("color"), sphereColor2.x(), sphereColor2.y(), sphereColor2.z(), 1.f);

			// --- Your Code Here ---
			// perform the query around this draw call.
			// Select which query to perform based on the mode - if it's SHOW_COUNT you should
			// count visible samples, and if it's SHOW_VISIBILITY you should just determine if 
			// the sphere is visible at all (boolean).
			sphereMesh.render();
			

			// --- Your Code Here ---
			// Get the results from the query object (based on mode) and
			// use them to update the text displayed.

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

		// --- Your Code Here ---
		// Remember to delete your query objects
	}



	gltDestroyText(text);
	gltTerminate();
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

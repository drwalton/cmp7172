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
#define GLT_IMPLEMENTATION
#include <gltext.h>
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include <opencv2/opencv.hpp>

/* In this exercise you should time rendering two different spheres, one low-poly and one high-poly
* using GL query objects.
* Time each of the render calls below, and show the results using GLT.
* Remember you'll need to check the results are available, probably waiting in a while loop
* until both queries are available to read.
* You can move around the meshes using the WASD keys, QE keys and mouse.
* Try zooming in on each sphere individually, or viewing them together, or from a distance.
* How does this affect the render times?
*/

const int winWidth = 1024, winHeight = 768;

const Uint64 desiredFrametime = 33;

void loadMesh(glhelper::Mesh* mesh, const std::string &filename) 
{
	Assimp::Importer importer;
	importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	const aiScene* aiscene = importer.GetScene();
	const aiMesh* aimesh = aiscene->mMeshes[0];

	std::vector<Eigen::Vector3f> verts(aimesh->mNumVertices);
	std::vector<Eigen::Vector3f> norms(aimesh->mNumVertices);
	std::vector<Eigen::Vector2f> uvs(aimesh->mNumVertices);
	std::vector<GLuint> elems(aimesh->mNumFaces*3);
	memcpy(verts.data(), aimesh->mVertices, aimesh->mNumVertices * sizeof(aiVector3D));
	memcpy(norms.data(), aimesh->mNormals, aimesh->mNumVertices * sizeof(aiVector3D));
	for (size_t v = 0; v < aimesh->mNumVertices; ++v) {
		uvs[v][0] = aimesh->mTextureCoords[0][v].x;
		uvs[v][1] = 1.f-aimesh->mTextureCoords[0][v].y;
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

Eigen::Matrix4f makeTranslationMatrix(const Eigen::Vector3f& translate)
{
	Eigen::Matrix4f matrix = Eigen::Matrix4f::Identity();
	matrix.block<3, 1>(0, 3) = translate;
	return matrix;
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
		viewer.position(Eigen::Vector3f(0.f, 0.f, -10.f));
		glhelper::Mesh lowPolyMesh, highPolyMesh;

        loadMesh(&lowPolyMesh, "../models/lowPolySphere.obj");
        loadMesh(&highPolyMesh, "../models/highPolySphere.obj");
		lowPolyMesh.shaderProgram(&fixedColorShader);
		highPolyMesh.shaderProgram(&fixedColorShader);
		lowPolyMesh.modelToWorld(makeTranslationMatrix(Eigen::Vector3f(-3.f, 0.f, 0.f)));
		highPolyMesh.modelToWorld(makeTranslationMatrix(Eigen::Vector3f(3.f, 0.f, 0.f)));

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// -- Your code here --
		// Create your GL query objects

		unsigned long long frameIdx = 0;

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
					if (event.key.keysym.sym == SDLK_1) {
						glEnable(GL_MULTISAMPLE);
					}
					else if (event.key.keysym.sym == SDLK_2) {
						glDisable(GL_MULTISAMPLE);
					}
				}
			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// -- Your code here --
			// Add gl query objects around these draw calls, and check
			// how long it takes to render each mesh. Display the information
			// on the GLT text by calling gltSetText().
			// To make things more readable, consider only updating the text every
			// second or so (30 frames).
			// It might also be easier if you convert the times from nanoseconds to
			// microseconds (divide by 1000).

			lowPolyMesh.render();
			highPolyMesh.render();

			gltBeginDraw();
			gltColor(1.f, 1.f, 1.f, 1.f);
			gltDrawText2D(text, 10.f, 10.f, 1.f);
			gltEndDraw();

			SDL_GL_SwapWindow(window);

			Uint64 elapsedFrameTime = SDL_GetTicks64() - frameStartTime;
			if (elapsedFrameTime < desiredFrametime) {
				SDL_Delay(desiredFrametime - elapsedFrameTime);
			}

			++frameIdx;
		}

		// -- Your code here --
		// Destroy your GL query objects
	}

	gltDestroyText(text);
	gltTerminate();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

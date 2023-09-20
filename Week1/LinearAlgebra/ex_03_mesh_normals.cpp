#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <iostream>
#include <exception>
#include <cmath>
#include "glhelper/ShaderProgram.hpp"
#include "glhelper/RotateViewer.hpp"
#include "glhelper/Mesh.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"

/* Exercise: Finding mesh normals
* The following code loads a mesh, and displays it with basic Gooch-style shading.
* The normals of the mesh are also rendered as little blue line segments.
* Try moving the camera around with the mouse and scroll wheel.
* 
* At the moment the normals are being calculated by Assimp using the 
* aiProcess_GenSmooth flag. I'd like you to calculate the normals using your own code instead.
* 
* Hints:
* ------
* 1. The cross product is a great way to find a normal to a triangle.
* 2. Multiple triangles meet at each vertex, so you'll want to average them (iterate over triangles, not vertices)
* 
* Once you have this working, try to weight the contributions of each triangle by its area.
* The idea here is that bigger triangles should affect the normal more, and smaller triangles less.
* 
* Hint: This might be easier than you think! You can use a property of the cross product here - remember 
* the length of the cross product is equal to an area...
*/

const int winWidth = 1024, winHeight = 720;

const Uint64 desiredFrametime = 33;

void loadSpotMesh(glhelper::Mesh* mesh) 
{
	Assimp::Importer importer;
	// --- Exercise ---
	// Change the line below to remove aiProcess_GenSmoothNormals
	importer.ReadFile("../models/spot/spot_triangulated.obj", aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	const aiScene* aiscene = importer.GetScene();
	const aiMesh* aimesh = aiscene->mMeshes[0];

	std::vector<Eigen::Vector3f> verts(aimesh->mNumVertices);
	std::vector<Eigen::Vector3f> norms(aimesh->mNumVertices);
	std::vector<GLuint> elems(aimesh->mNumFaces*3);
	memcpy(verts.data(), aimesh->mVertices, aimesh->mNumVertices * sizeof(aiVector3D));
	for (size_t f = 0; f < aimesh->mNumFaces; ++f) {
		for (size_t i = 0; i < 3; ++i) {
			elems[f * 3 + i] = aimesh->mFaces[f].mIndices[i];
		}
	}

	// --- Exercise ---
	// delete the line below, and populate norms with normals you calculate yourself.
	memcpy(norms.data(), aimesh->mNormals, aimesh->mNumVertices * sizeof(aiVector3D));


	mesh->vert(verts);
	mesh->norm(norms);
	mesh->elems(elems);
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

	{
		glhelper::ShaderProgram goochShader({ "../shaders/Gooch.vert", "../shaders/Gooch.frag" });
		glhelper::ShaderProgram drawNormalShader({ "../shaders/DrawNormals.vert", "../shaders/DrawNormals.geom", "../shaders/DrawNormals.frag" });
		glProgramUniform4f(drawNormalShader.get(), drawNormalShader.uniformLoc("normColor"), 0.0f, 0.5f, 0.5f, 1.0f);
		glProgramUniform1f(drawNormalShader.get(), drawNormalShader.uniformLoc("lineWidth"), 0.01f);
		glProgramUniform1f(drawNormalShader.get(), drawNormalShader.uniformLoc("lineLength"), 0.1f);
		glhelper::RotateViewer viewer(winWidth, winHeight);
		glhelper::Mesh mesh;

		loadSpotMesh(&mesh);

		mesh.shaderProgram(&goochShader);


		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);

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

			mesh.shaderProgram(&goochShader);
			mesh.drawMode(GL_TRIANGLES);
			mesh.render();

			mesh.shaderProgram(&drawNormalShader);
			mesh.drawMode(GL_POINTS);
			mesh.render();
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


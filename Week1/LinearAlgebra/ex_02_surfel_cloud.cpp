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

/* Exercise: Drawing Surfels
* Surfels are an alternative way of representing 3D shapes to triangle meshes.
* They are commonly used in 3D reconstruction, and are a variant of point clouds
* where each point also has a normal and radius.
*
* OpenGL can't display them directly, so I'd like you to convert this surfel cloud
* into a series of triangles and show it using the viewer code provided.
* Each surfel should be drawn as a square consisting of two triangles. The square
* should be perpendicular to the normal, and each of its side lengths should be 
* equal to the surfelSize variable.
* 
* At the moment, the code just draws the normals to each surfel as little blue lines -
* you can use these to check that your surfels are in the right place, and oriented correctly.
* 
* Hints:
* ------
* 1. Use the cross product to find vectors perpendicular to each other, and to the specified
*    normal.
*/

const int winWidth = 1024, winHeight = 720;

const Uint64 desiredFrametime = 33;

void loadNormalMesh(glhelper::Mesh* mesh) 
{
	Assimp::Importer importer;
	// --- Exercise ---
	// Change the line below to remove aiProcess_GenSmoothNormals
	importer.ReadFile("../models/wheel.ply", 0);
	const aiScene* aiscene = importer.GetScene();
	const aiMesh* aimesh = aiscene->mMeshes[0];

	std::vector<Eigen::Vector3f> verts(aimesh->mNumVertices);
	std::vector<Eigen::Vector3f> norms(aimesh->mNumVertices);

	// --- Exercise ---
	// delete the line below, and populate norms with normals you calculate yourself.
	memcpy(norms.data(), aimesh->mNormals, aimesh->mNumVertices * sizeof(aiVector3D));


	mesh->vert(verts);
	mesh->norm(norms);
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
	// Turns on 4x MSAA (remember to glEnable(GL_MULTISAMPLE))
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
		glhelper::ShaderProgram drawNormalShader({ "../shaders/DrawNormals.vert", "../shaders/DrawNormals.geom", "../shaders/DrawNormals.frag" });
		glhelper::ShaderProgram goochShader({ "../shaders/Gooch.vert", "../shaders/Gooch.frag" });
		glProgramUniform4f(drawNormalShader.get(), drawNormalShader.uniformLoc("normColor"), 0.0f, 0.5f, 0.5f, 1.0f);
		glProgramUniform1f(drawNormalShader.get(), drawNormalShader.uniformLoc("lineWidth"), 0.01f);
		glProgramUniform1f(drawNormalShader.get(), drawNormalShader.uniformLoc("lineLength"), 0.1f);
		glhelper::RotateViewer viewer(winWidth, winHeight);
		glhelper::Mesh normalMesh, surfelMesh;

		Assimp::Importer importer;
		importer.ReadFile("../models/wheel.ply", 0);
		const aiScene* aiscene = importer.GetScene();
		const aiMesh* aimesh = aiscene->mMeshes[0];

		std::vector<Eigen::Vector3f> verts(aimesh->mNumVertices);
		std::vector<Eigen::Vector3f> norms(aimesh->mNumVertices);

		memcpy(verts.data(), aimesh->mVertices, aimesh->mNumVertices * sizeof(aiVector3D));
		for (auto& vert : verts) vert *= 0.1f;
		memcpy(norms.data(), aimesh->mNormals, aimesh->mNumVertices * sizeof(aiVector3D));


		normalMesh.vert(verts);
		normalMesh.norm(norms);
		normalMesh.drawMode(GL_POINTS);

		float surfelSize = 0.05f;
		//-- Exercise --
		// From the verts and norms above, generate the surfel geometry as specified above and store it in
		// the surfelMesh.
		std::vector<Eigen::Vector3f> surfelVerts;
		std::vector<Eigen::Vector3f> surfelNorms;

		// ---- Your Code Here ----

		surfelMesh.vert(surfelVerts);
		surfelMesh.norm(surfelNorms);

		normalMesh.shaderProgram(&drawNormalShader);
		surfelMesh.shaderProgram(&goochShader);

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

			normalMesh.render();
			glDisable(GL_CULL_FACE);
			surfelMesh.render();

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


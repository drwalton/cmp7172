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

/* Exercise: Transforming mesh normals
* The following code loads a mesh, and displays it with basic Gooch-style shading.
* The normals of the mesh are also rendered as little blue line segments.
* Try moving the camera around with the mouse and scroll wheel.

* At the moment the normals of the mesh are also being transformed by the same modelToWorld
* matrix applied to the vertices. This is clearly wrong! If you set translateMode to true (press SPACE),
* the sphere will circle around. Note the position of the cow in space affects the normals (this is incorrect)
* Also note that if you turn off translateMode the normals are affected in a weird way by 
* the scaling.
* 
* Correct the findNormalTransformationMatrix function to output the right matrix to transform
* the normals with. Check the normals look correct with translateMode true and translateMode false;
*/

const int winWidth = 1024, winHeight = 720;

const Uint64 desiredFrametime = 33;

float theta = 0.f;
int translateMode = 0;

void loadMesh(glhelper::Mesh* mesh) 
{
	Assimp::Importer importer;
	importer.ReadFile("../models/sphere.obj", aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	const aiScene* aiscene = importer.GetScene();
	const aiMesh* aimesh = aiscene->mMeshes[0];

	std::vector<Eigen::Vector3f> verts(aimesh->mNumVertices);
	std::vector<Eigen::Vector3f> norms(aimesh->mNumVertices);
	std::vector<GLuint> elems(aimesh->mNumFaces*3);
	memcpy(verts.data(), aimesh->mVertices, aimesh->mNumVertices * sizeof(aiVector3D));
	memcpy(norms.data(), aimesh->mNormals, aimesh->mNumVertices * sizeof(aiVector3D));
	for (size_t f = 0; f < aimesh->mNumFaces; ++f) {
		for (size_t i = 0; i < 3; ++i) {
			elems[f * 3 + i] = aimesh->mFaces[f].mIndices[i];
		}
	}

	mesh->vert(verts);
	mesh->norm(norms);
	mesh->elems(elems);
}

Eigen::Matrix4f findNormalTransformationMatrix(const Eigen::Matrix4f& modelToWorldMatrix)
{
	// ----- Your code here -----
	// Correct this to output an appropriate matrix to transform the normals with.
	// Requirements:
	// Translation doesn't affect the normals.
	// Normals respond correctly to model scaling.
	return modelToWorldMatrix;
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
		glhelper::ShaderProgram drawNormalShader({ "../shaders/DrawNormalsSetNormMat.vert", "../shaders/DrawNormals.geom", "../shaders/DrawNormals.frag" });
		//glhelper::ShaderProgram drawNormalShader({ "../shaders/DrawNormals.vert", "../shaders/DrawNormals.geom", "../shaders/DrawNormals.frag" });
		glProgramUniform4f(drawNormalShader.get(), drawNormalShader.uniformLoc("normColor"), 0.0f, 0.5f, 0.5f, 1.0f);
		glProgramUniform1f(drawNormalShader.get(), drawNormalShader.uniformLoc("lineWidth"), 0.01f);
		glProgramUniform1f(drawNormalShader.get(), drawNormalShader.uniformLoc("lineLength"), 0.1f);
		glhelper::RotateViewer viewer(winWidth, winHeight);
		glhelper::Mesh mesh;
		loadMesh(&mesh);

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
				if (event.type == SDL_KEYDOWN &&
					event.key.keysym.sym == SDLK_SPACE)
				{
					translateMode += 1;
					translateMode %= 3;
				}
			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			Eigen::Matrix4f modelToWorldMatrix;

			if (translateMode == 0) {
				modelToWorldMatrix <<
					1.f, 0.f, 0.f, 0.5f * cos(theta),
					0.f, 1.f, 0.f, 0.5f * sin(theta),
					0.f, 0.f, 1.f, 0.f,
					0.f, 0.f, 0.f, 1.f;
			}
			else if (translateMode == 1) {
				modelToWorldMatrix <<
					1.f, 0.5f + 0.5f*cos(theta), 0.f, 0.f,
					0.f, 1.f, 0.f, 0.f,
					0.f, 0.f, 1.f, 0.f,
					0.f, 0.f, 0.f, 1.f;

			}
			else {
				modelToWorldMatrix <<
					1.f + 0.5f*sin(theta), 0.f, 0.f, 0.f,
					0.f, 1.f, 0.f, 0.f,
					0.f, 0.f, 1.f, 0.f,
					0.f, 0.f, 0.f, 1.f;
			}

			Eigen::Matrix4f normToWorldMatrix = findNormalTransformationMatrix(modelToWorldMatrix);

			mesh.modelToWorld(modelToWorldMatrix);
			glProgramUniformMatrix4fv(drawNormalShader.get(), drawNormalShader.uniformLoc("myNormToWorld"), 1, GL_FALSE, normToWorldMatrix.data());

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

			theta += 0.01f;
			if (theta > 2 * M_PI) theta -= 2 * M_PI;
		}
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}


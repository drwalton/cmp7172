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

/* This exercise will be a quick reminder how to load and use textures in shaders in OpenGL
* and a chance to load a texture using OpenCV. 
* You should change the code to load the texture image as indicated, create an OpenGL texture
* and copy the image data over, and finally bind the GL texture to the shader so that it is
* displayed. Your final output should look similar to image/ex_00_texturing.png.
*/

const int winWidth = 1024, winHeight = 768;

const Uint64 desiredFrametime = 33;

void loadSpotMesh(glhelper::Mesh* mesh) 
{
	Assimp::Importer importer;
	importer.ReadFile("../models/spot/spot_triangulated.obj", aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	const aiScene* aiscene = importer.GetScene();
	const aiMesh* aimesh = aiscene->mMeshes[0];

	std::vector<Eigen::Vector3f> verts(aimesh->mNumVertices);
	std::vector<Eigen::Vector3f> norms(aimesh->mNumVertices);
	std::vector<Eigen::Vector2f> uvs(aimesh->mNumVertices);
	std::vector<GLuint> elems(aimesh->mNumFaces*3);
	memcpy(verts.data(), aimesh->mVertices, aimesh->mNumVertices * sizeof(aiVector3D));
	memcpy(norms.data(), aimesh->mNormals, aimesh->mNumVertices * sizeof(aiVector3D));
	// It would be nice to just memcpy here too, but Assimp stores texture coordinates
	// as 3-vectors (this makes sense, as in principle you could sample a 3D texture)
	// We just get the x and y components of the texture coords.
	// Note we have to flip the y coordinate relative to Assimp too.
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
		glhelper::ShaderProgram texturedMeshShader({ "../shaders/TexturedMesh.vert", "../shaders/TexturedMesh.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);
		//glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh spotMesh;

        loadSpotMesh(&spotMesh);
		spotMesh.shaderProgram(&texturedMeshShader);

		// --- Your code here ---
		// Here you should load the texture image from ../models/spot/spot_texture.png using OpenCV
		// Create an OpenGL texture from this image (use glGenTextures, glTexImage2D etc.)
		// Also, make sure to set the sampler uniform (called tex) in the texturedMeshShader
		// to match the image unit you plan to use (you only have one texture, so you can set it to 0).
		// Hints
		// -----
		// 1. If your texture looks black, it may be because you haven't generated MIP maps for your texture
		//    you can do this quickly by calling glGenerateTextureMipmap after creating it.
		// 2. If you get weird colours, remember OpenCV images come in BGR format by default. 
		//    You might need to swap it to RGB before providing it to OpenGL (use cv::cvtColor()).

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

			glDisable(GL_CULL_FACE);

			// --- Your code here ---
			// Before rendering the Spot mesh, bind the texture you created to the
			// correct image unit (probably 0, so GL_TEXTURE0)

			SDL_GL_SwapWindow(window);

			Uint64 elapsedFrameTime = SDL_GetTicks64() - frameStartTime;
			if (elapsedFrameTime < desiredFrametime) {
				SDL_Delay(desiredFrametime - elapsedFrameTime);
			}
		}

		// --- Your code here ---
		// Make sure to delete the texture you created here.
	}


	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

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
#include "glhelper/Texture.hpp"
#include "glhelper/Matrices.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include <opencv2/opencv.hpp>
#define GLT_IMPLEMENTATION
#include <gltext.h>
/* In this exercise you'll implement the "Wrap Lighting" method used to simulate
* larger light sources on diffuse surfaces covered in the lecture.
* The current code shows the Spot mesh with a colour texture, but has no lighting
* code. You should revise the WrapLighting.frag shader to implement the technique.
* The user should be able to use some controls (e.g. up and down arrows) to adjust
* the effect and go from point source to full hemispherical light source.
* The mesh should be illuminated by a light source at the location of the white 
* sphere in the scene. Make sure to scale your light intensity by 1/distance^2.
* Pressing the space bar causes the light sphere to rotate - make sure your lighting
* also updates in real time to match!
* 
*/

const int winWidth = 640, winHeight = 480;

const Uint64 desiredFrametime = 33;

float wrapAmount = 0.0f;
float theta = 0.0f;
bool lightRotating = false;
Eigen::Vector3f lightPos(5.f*sinf(theta), 5.f, 5.f*cosf(theta));

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

	gltInit();
	GLTtext* text = gltCreateText();
	gltSetText(text, (std::string("wrapAmount: ") + std::to_string(wrapAmount)).c_str());
	glEnable(GL_MULTISAMPLE);

	{
		glhelper::ShaderProgram wrapLightingShader({ "../shaders/WrapLighting.vert", "../shaders/WrapLighting.frag" });
		glhelper::ShaderProgram fixedColorShader({ "../shaders/FixedColor.vert", "../shaders/FixedColor.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);
		//glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh bunnyMesh, sphereMesh;

		Eigen::Matrix4f bunnyModelToWorld = Eigen::Matrix4f::Identity();
		bunnyModelToWorld(0, 0) = 0.1f;
		bunnyModelToWorld(1, 1) = 0.1f;
		bunnyModelToWorld(2, 2) = 0.1f;
		bunnyModelToWorld = makeTranslationMatrix(Eigen::Vector3f(0.f, -0.5f, 0.f)) * bunnyModelToWorld;

        loadMesh(&bunnyMesh, "../models/stanford_bunny/scene.gltf");
        loadMesh(&sphereMesh, "../models/sphere.obj");
		bunnyMesh.modelToWorld(bunnyModelToWorld);
		bunnyMesh.shaderProgram(&wrapLightingShader);
		sphereMesh.shaderProgram(&fixedColorShader);
		sphereMesh.modelToWorld(makeTranslationMatrix(lightPos));

		glProgramUniform1i(wrapLightingShader.get(), wrapLightingShader.uniformLoc("albedoTexture"), 0);
		glProgramUniform4f(fixedColorShader.get(), fixedColorShader.uniformLoc("color"), 1.f, 1.f, 1.f, 1.f);

		cv::Mat bunnyTextureImage = cv::imread("../models/stanford_bunny/textures/Bunny_baseColor.png");
		cv::cvtColor(bunnyTextureImage, bunnyTextureImage, cv::COLOR_BGR2RGB);
		glhelper::Texture bunnyTexture(
			GL_TEXTURE_2D, GL_RGB8, 
			bunnyTextureImage.cols, bunnyTextureImage.rows, 
			0, GL_RGB, GL_UNSIGNED_BYTE, bunnyTextureImage.data,
			GL_LINEAR_MIPMAP_LINEAR,
			GL_LINEAR);
		bunnyTexture.genMipmap();


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
					if (event.key.keysym.sym == SDLK_SPACE) {
						lightRotating = !lightRotating;
					}
				}

				// --- Your code here ---
				// You could add your controls to adjust the wrapLighting effect here
			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (lightRotating) {
				theta += 0.01f;
				if (theta > 2 * 3.14159f) theta = 0.f;
				lightPos << 5.f * sinf(theta), 5.f, 5.f * cosf(theta);
				sphereMesh.modelToWorld(makeTranslationMatrix(lightPos));
			}

			glDisable(GL_CULL_FACE);
			bunnyTexture.bindToImageUnit(0);
			bunnyMesh.render();
			sphereMesh.render();

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
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

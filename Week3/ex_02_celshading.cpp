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
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include <opencv2/opencv.hpp>
#define GLT_IMPLEMENTATION
#include <gltext.h>
/* In this exercise you'll implement a Cel Shading-style Non-Photorealistic
* Rendering technique. This will use the dot product of the light direction
* with the normal to look up a suitable intensity in a 1D texture. There are
* a few 1D images in the images/ directory for you to try - you can also try
* creating some of your own in your image editor of choice!
* 
* You can press the space bar to make the light rotate to preview your 
* lighting.
* 
* Tasks
* -----
* 
* 1. Create the 1D texture, ready to use in the shader. The process will
* be similar to creating a 2D texture but adapt the parameters you give
* to glTexImage2D appropriately (check the documentation).
* 2. Adapt the shader CelShading.frag to look up the intensity value based
* on n.l in the texture, and modulate the albedo texture accordingly.
* 
* Hints
* -----
* 
* The wrapping mode of your texture might be important here - if you have 
* reflection wrapping turned on you might get weird results when n.l is
* close to 0 or 1.
* Remember to clamp n.l too!
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

	gltInit();
	GLTtext* text = gltCreateText();
	gltSetText(text, "Cel Shading Example");
	glEnable(GL_MULTISAMPLE);

	{
		glhelper::ShaderProgram celShadingShader({ "../shaders/CelShading.vert", "../shaders/CelShading.frag" });
		glhelper::ShaderProgram fixedColorShader({ "../shaders/FixedColor.vert", "../shaders/FixedColor.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);
		//glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh spotMesh, sphereMesh;

        loadMesh(&spotMesh, "../models/spot/spot_triangulated.obj");
        loadMesh(&sphereMesh, "../models/sphere.obj");
		spotMesh.shaderProgram(&celShadingShader);
		sphereMesh.shaderProgram(&fixedColorShader);
		sphereMesh.modelToWorld(makeTranslationMatrix(lightPos));

		glProgramUniform1i(celShadingShader.get(), celShadingShader.uniformLoc("albedoTexture"), 0);
		glProgramUniform1i(celShadingShader.get(),celShadingShader.uniformLoc("lightingTexture"), 1);
		glProgramUniform3f(celShadingShader.get(), celShadingShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());
		glProgramUniform4f(fixedColorShader.get(), fixedColorShader.uniformLoc("color"), 1.f, 1.f, 1.f, 1.f);

		cv::Mat spotTextureImage = cv::imread("../models/spot/spot_texture.png");
		cv::cvtColor(spotTextureImage, spotTextureImage, cv::COLOR_BGR2RGB);
		GLuint spotTexture;
		glGenTextures(1, &spotTexture);
		glBindTexture(GL_TEXTURE_2D, spotTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
			spotTextureImage.cols, spotTextureImage.rows,
			0, GL_RGB, GL_UNSIGNED_BYTE, spotTextureImage.data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateTextureMipmap(spotTexture);

		// --- Your Code Here ---
		// Load one of the 1D lighting images (e.g. ../images/cel_texture_2level.png)
		// Load it as a 1D texture (use GL_TEXTURE_1D)
		// Some of the settings will need to be adjusted from using 2D textures
		// like you did in the last lab - do check the specs!

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
			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (lightRotating) {
				theta += 0.01f;
				if (theta > 2 * 3.14159f) theta = 0.f;
				lightPos << 5.f * sinf(theta), 5.f, 5.f * cosf(theta);
				sphereMesh.modelToWorld(makeTranslationMatrix(lightPos));
				glProgramUniform3f(celShadingShader.get(), celShadingShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());
			}

			glDisable(GL_CULL_FACE);
			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, spotTexture);
			// -- Your Code Here --
			// Bind your new 1D lighting texture to image unit 1.

			spotMesh.render();
			sphereMesh.render();

			// -- Your Code Here --
			// For the optional extra task, render spotMesh again with backface shell
			// expansion to make some nice black outlines.
			// Remember to change the culling settings to cull frontfaces rather 
			// than backfaces, and change them back afterwards!

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
		glDeleteTextures(1, &spotTexture);
	}

	gltDeleteText(text);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

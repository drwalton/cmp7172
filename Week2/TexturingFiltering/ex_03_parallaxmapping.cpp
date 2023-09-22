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
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include <chrono>

/* In this exercise you'll implement a range of parallax mapping techniques that we discussed in the earlier
* lecture. 
* The existing application shows a plane textured with an image of cobblestones. You can enable/disable normal mapping
* by pressing the N key. You should implement the specified parallax mapping techniques so you can swap between and test
* them with the P key. Pressing the up and down arrows adjusts a depthScale uniform which I recommend you use in 
* your implementation in ParallaxMap.frag to scale the depth of the parallax map.
* 
* You should implement the following techniques:
* 1. Regular parallax mapping
* 2. Offset-limited parallax mapping
* 3. Steep parallax mapping
*
* See images/exercise_03_*.png for examples of expected output.
* 
* If you have time, also try implementing parallax occlusion mapping!
*
* Note
* ----
* The normal mapping I've implemented here cheats and just uses the normals from the map directly
* rather than transforming to tangent space. I'm only able to do this because we're texturing a plane
* and generally you should transform as you did in the previous exercise.
* 
* Hints
* -----
* 
* In this exercise you'll probably find it much easier to work in tangent space, rather than world space.
* In the previous exercise you probably used the TBN matrix to go from tangent to world space.
* This time you'll want to map e.g. camera direction from world space to tangent space.
* You can use the inverse of TBN. If you've computed it correctly TBN should be orthogonal.
* Is there a quick way to find the inverse of an orthogonal matrix?
*/

const int winWidth = 1024, winHeight = 768;

const Uint64 desiredFrametime = 33;
// Sets the parallax mode.
// 0: Regular parallax mapping
// 1: Offset-limited parallax mapping
// 2: Steep parallax mapping
// 3: Parallax mapping off
int parallaxMode = 0;

// Normal mapping mode
// 0 off
// 1 on
int normalMapMode = 1;

float depthScale = 0.05;
float theta = 0.0f;

GLuint createTexture(const cv::Mat& image, bool grayscale)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	if(grayscale)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, image.cols, image.rows, 0, GL_RED, GL_UNSIGNED_BYTE, image.data);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);


	glGenerateTextureMipmap(texture);
	
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// This isn't strictly necessary as GL_REPEAT is the default mode.
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return texture;
}

void updateText(GLTtext* text)
{
	std::stringstream textStream;


	if (parallaxMode == 0) {
		textStream << "Parallax Mapping On, Scale " << depthScale;
	} 
	else if (parallaxMode == 1) {
		textStream << "Offset-Limited Parallax Mapping On, Scale " << depthScale;
	}
	else if (parallaxMode == 2) {
		textStream << "Steep Parallax Mapping On, Scale " << depthScale;
	}
	else {
		textStream << "Parallax Mapping Off";
	}
	if (normalMapMode == 1) {
		textStream << " Normal Mapping On";
	}
	else {
		textStream << " Normal Mapping Off";
	}
	gltSetText(text, textStream.str().c_str());
}

void loadSphereMesh(glhelper::Mesh* mesh) 
{
	Assimp::Importer importer;
	importer.ReadFile("../models/sphere.obj", aiProcess_Triangulate);
	const aiScene* aiscene = importer.GetScene();
	const aiMesh* aimesh = aiscene->mMeshes[0];

	std::vector<Eigen::Vector3f> verts(aimesh->mNumVertices);
	std::vector<GLuint> elems(aimesh->mNumFaces*3);
	memcpy(verts.data(), aimesh->mVertices, aimesh->mNumVertices * sizeof(aiVector3D));
	for (size_t f = 0; f < aimesh->mNumFaces; ++f) {
		for (size_t i = 0; i < 3; ++i) {
			elems[f * 3 + i] = aimesh->mFaces[f].mIndices[i];
		}
	}

	mesh->vert(verts);
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

	// Prepare window
	SDL_Window* window = SDL_CreateWindow("A test window", 50, 50, winWidth, winHeight, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);


	GLenum result = glewInit();
	if (result != GLEW_OK) {
		throw std::runtime_error("GLEW couldn't initialize.");
	}
	if (!GLEW_EXT_timer_query) {
		std::cout << "No timer query extension";
	}


	gltInit();
	GLTtext* text = gltCreateText();
	updateText(text);

	{
		glhelper::ShaderProgram parallaxMapShader({ "../shaders/ParallaxMap.vert", "../shaders/ParallaxMap.frag" });
		glhelper::ShaderProgram lightShader({ "../shaders/FixedColor.vert", "../shaders/FixedColor.frag" });
		glProgramUniform4f(lightShader.get(), lightShader.uniformLoc("color"), 1.f, 1.f, 1.f, 1.f);
		glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh planeMesh, lightMesh;
		loadSphereMesh(&lightMesh);
		lightMesh.shaderProgram(&lightShader);
		Eigen::Vector3f lightPos(10.f, 10.f, 0.f);
		Eigen::Matrix4f lightToWorld = Eigen::Matrix4f::Identity();
		lightToWorld.block<3, 1>(0, 3) = lightPos;
		lightMesh.modelToWorld(lightToWorld);


		float planeSize = 10.f;

		auto t1 = std::chrono::steady_clock::now();
		std::vector<Eigen::Vector3f> planePoints{
			Eigen::Vector3f(-planeSize * 0.5f, 0.f, -planeSize * 0.5f),
			Eigen::Vector3f(planeSize * 0.5f, 0.f, -planeSize * 0.5f),
			Eigen::Vector3f(-planeSize * 0.5f, 0.f, planeSize * 0.5f),
			Eigen::Vector3f(planeSize * 0.5f, 0.f, planeSize * 0.5f),
		};
		std::vector<Eigen::Vector2f> planeTexCoords{
			Eigen::Vector2f(0.f, 0.f),
			Eigen::Vector2f(1.f, 0.f),
			Eigen::Vector2f(0.f, 1.f),
			Eigen::Vector2f(1.f, 1.f),
		};
		std::vector<Eigen::Vector3f> planeNorms(4), planeTangents(4), planeBitangents(4);

		for (size_t i = 0; i < 4; ++i) {
			planeNorms[i] = Eigen::Vector3f(0.f, 1.f, 0.f);
			planeTangents[i] = Eigen::Vector3f(1.f, 0.f, 0.f);
			planeBitangents[i] = Eigen::Vector3f(0.f, 0.f, 1.f);
		}

		auto diff = std::chrono::steady_clock::now() - t1;
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << std::endl;

		std::vector<GLuint> planeElems{ 0,1,2,1,2,3 };

		planeMesh.vert(planePoints);
		planeMesh.tex(planeTexCoords);
		planeMesh.elems(planeElems);
		planeMesh.norm(planeNorms);
		planeMesh.tangent(planeTangents);
		planeMesh.bitangent(planeBitangents);

		planeMesh.shaderProgram(&parallaxMapShader);
		Eigen::Matrix4f planeToWorld = Eigen::Matrix4f::Identity();
		planeToWorld.block<3, 1>(0, 3) = Eigen::Vector3f(0.f, -3.f, 0.f);
		planeMesh.modelToWorld(planeToWorld);


		glProgramUniform1i(parallaxMapShader.get(), parallaxMapShader.uniformLoc("albedoTex"), 0);
		glProgramUniform1i(parallaxMapShader.get(), parallaxMapShader.uniformLoc("depthTex"), 1);
		glProgramUniform1i(parallaxMapShader.get(), parallaxMapShader.uniformLoc("normalTex"), 2);
		glProgramUniform1i(parallaxMapShader.get(), parallaxMapShader.uniformLoc("parallaxMode"), parallaxMode);
		glProgramUniform1i(parallaxMapShader.get(), parallaxMapShader.uniformLoc("normalMapMode"), normalMapMode);
		glProgramUniform1f(parallaxMapShader.get(), parallaxMapShader.uniformLoc("depthScale"), depthScale);
		glProgramUniform3f(parallaxMapShader.get(), parallaxMapShader.uniformLoc("lightPos"), lightPos.x(), lightPos.y(), lightPos.z());

		GLuint albedoTexture, depthTexture, normalTexture;

		{
			cv::Mat depthmap = cv::imread("../images/cobblestone_depth.png");
			cv::cvtColor(depthmap, depthmap, cv::COLOR_BGR2GRAY);
			std::cout << depthmap.rows << " " << depthmap.cols << std::endl;
			depthTexture = createTexture(depthmap, true);

			cv::Mat normalmap = cv::imread("../images/cobblestone_normal.png");
			cv::cvtColor(normalmap, normalmap, cv::COLOR_BGR2RGB);
			normalTexture = createTexture(normalmap, false);

			cv::Mat image = cv::imread("../images/cobblestone_albedo.png");
			std::cout << image.rows << " " << image.cols << " " << image.channels() << std::endl;
			cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
			albedoTexture = createTexture(image, false);

		}


		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);

		while (!shouldQuit) {
			Uint64 frameStartTime = SDL_GetTicks64();
			viewer.update();

			while (SDL_PollEvent(&event)) {
				// Check for X of window being clicked, or ALT+F4
				if (event.type == SDL_QUIT) {
					shouldQuit = true;
				}
				if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.sym == SDLK_p) {
						parallaxMode += 1;
						parallaxMode %= 4;
						glProgramUniform1i(parallaxMapShader.get(), parallaxMapShader.uniformLoc("parallaxMode"), parallaxMode);
						updateText(text);
					}
					if (event.key.keysym.sym == SDLK_n) {
						normalMapMode += 1;
						normalMapMode %= 2;
						glProgramUniform1i(parallaxMapShader.get(), parallaxMapShader.uniformLoc("normalMapMode"), normalMapMode);
						updateText(text);
					}
					if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN) {
						if (event.key.keysym.sym == SDLK_UP) {
							depthScale += 0.01;
						}
						if (event.key.keysym.sym == SDLK_DOWN) {
							depthScale -= 0.01;
						}
						glProgramUniform1f(parallaxMapShader.get(), parallaxMapShader.uniformLoc("depthScale"), depthScale);
						updateText(text);
					}
				}
				viewer.processEvent(event);
			}

			//theta += 0.01f;
			if (theta > 2 * 3.14159) theta -= 2 * 3.14159;
			auto currModelToWorld = planeMesh.modelToWorld();
			currModelToWorld.block<3,3>(0,0) = Eigen::AngleAxisf(theta, Eigen::Vector3f(0.f, 1.f, 0.f)).matrix();
			planeMesh.modelToWorld(currModelToWorld);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDisable(GL_CULL_FACE);
			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, albedoTexture);
			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, normalTexture);

			planeMesh.render();
			lightMesh.render();

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

		glDeleteTextures(1, &albedoTexture);
		glDeleteTextures(1, &depthTexture);
	}


	gltDeleteText(text);
	gltTerminate();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

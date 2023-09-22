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

/*  In this exercise if you start the code you'll see we're rendering a moon orbiting
* the earth. When the moon is hidden behind the earth, we'd like to save rendering time
* by not drawing it. We can use conditional rendering for this!
* 
* Draw a sphere the same size & shape as the moon first, using the basic fixedColorShader. 
* Perform an occlusion query as you did in the previous exercise, and only render the moon
* with the full texture if it's visible (using conditional rendering).
* 
* To check it's actually saving time, you should allow conditional rendering to be turned 
* on/off using the space bar, and time the rendering of the moon (just the full textured
* moon, not the visibility test). Add your render time to the text displayed at the top
* of the screen. When the moon is hidden behind the earth, but still in the viewport, 
* your render time should drop to very little when conditional rendering is used.
* 
* Hints
* -----
* If you can only see your test mesh, and not the moon, remember that drawing the test
* mesh will add its depths to the Z-buffer. This will mean the actual moon with the nice
* texture will fail the depth test. Disable writing depths to the depth buffer when 
* drawing the test mesh (but keep depth testing on!)
* 
* If the moon rotating is getting annoying you can start/stop it with the M key.
*/

const int winWidth = 1024, winHeight = 768;

const Uint64 desiredFrametime = 33;

bool conditionalRenderingOn = false;
float moonTheta = 0.f;
const float moonRadius = 6.f;
bool moonRotating = true;

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

GLuint createTexture(const cv::Mat& image)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);

	glGenerateTextureMipmap(texture);

	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// This isn't strictly necessary as GL_REPEAT is the default mode.
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return texture;
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
		glhelper::ShaderProgram texturedMeshLambertShader({ "../shaders/TexturedMeshLambert.vert", "../shaders/TexturedMeshLambert.frag" });
		glProgramUniform4f(fixedColorShader.get(), fixedColorShader.uniformLoc("color"), 0.1f, 0.8f, 0.8f, 1.0f);
		glProgramUniform1i(texturedMeshLambertShader.get(), texturedMeshLambertShader.uniformLoc("tex"), 0);
		//glhelper::RotateViewer viewer(winWidth, winHeight);
		glhelper::FlyViewer viewer(winWidth, winHeight);
		viewer.position(Eigen::Vector3f(0.f, 0.f, -15.f));
		glhelper::Mesh sphereMesh;

		auto image = cv::imread("../images/lroc_color_poles_2k.tif");
		cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
		GLuint moonTexture = createTexture(image);

		image = cv::imread("../images/ear0xuu2.tif");
		cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
		GLuint earthTexture = createTexture(image);

        loadMesh(&sphereMesh, "../models/highPolySphere.obj");
		sphereMesh.shaderProgram(&fixedColorShader);

		Eigen::Matrix4f earthMatrix, moonPosMatrix;
		earthMatrix <<
			3.f, 0.f, 0.f, 0.f,
			0.f, 3.f, 0.f, 0.f,
			0.f, 0.f, 3.f, 0.f,
			0.f, 0.f, 0.f, 1.f;
		moonPosMatrix = Eigen::Matrix4f::Identity();
		moonPosMatrix.block<3, 1>(0, 3) = Eigen::Vector3f(-moonRadius, 0.f, 0.f);

		Eigen::Vector4f testColor = Eigen::Vector4f(0.1f, 0.8f, 0.8f, 1.f);


		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// --- Your Code Here ---
		// Make the query objects you need for timing and visibility

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
						conditionalRenderingOn = !conditionalRenderingOn;
					}
					if (event.key.keysym.sym == SDLK_m) {
						moonRotating = !moonRotating;
					}
				}
			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDisable(GL_CULL_FACE);

			sphereMesh.shaderProgram(&texturedMeshLambertShader);
			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, earthTexture);
			sphereMesh.modelToWorld(earthMatrix);
			sphereMesh.render();

			if (moonRotating) moonTheta += 0.01f;
			if (moonTheta > 2.f * 3.14159f) moonTheta -= 2.f * 3.14159f;

			Eigen::Matrix3f moonRot3 = Eigen::AngleAxisf(moonTheta, Eigen::Vector3f(0.f, 1.f, 0.f)).matrix();
			Eigen::Matrix4f moonRot = Eigen::Matrix4f::Identity();
			moonRot.block<3, 3>(0, 0) = moonRot3;

			sphereMesh.modelToWorld(moonRot * moonPosMatrix);

			// --- Your Code Here ---
			// Render a version of the mesh using just the fixedColorShader
			// and perform a visibility query.
			// Be careful about depth values!


			// --- Your Code Here ----
			// Make the rendering of the moon here conditional depending on
			// the previous visibility query, if conditionalRenderingOn is true.
			// Otherwise, don't do conditional rendering and always render the moon.
			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, moonTexture);
			sphereMesh.shaderProgram(&texturedMeshLambertShader);
			sphereMesh.render();

			
			// --- Your Code Here --- 
			// Set the text to display the important info 
			// If the moon is visible or not?
			// Is conditional rendering on or off?
			// How long did it take to render the nice textured moon?

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
		// Delete your query objects.

		glDeleteTextures(1, &moonTexture);
		glDeleteTextures(1, &earthTexture);
	}



	gltDestroyText(text);
	gltTerminate();
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <iostream>
#include <exception>
#include <cmath>
#include <random>
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
/* In this exercise you'll use geometry shaders to run a simple particle-based fire simulation.
* Currently the code renders a set of points with GL_POINTS. The vertex shader moves each point
* along a cubic curve, which is fitted to some parameters for the fire given below.
* The geometry shader you will add will take each of these points and generate a quad, orienting it
* to face towards the camera. In the fragment shader you'll combine samples from a 1D flame colour texture
* and a 2D flame transparency (alpha) texture to provide colour to the flame. To add movement
* you'll scroll the sampling window along the alpha texture.
* 
* Tasks
* =====
* 
* 1. Write the geometry shader FireParticle.geom. This should take the point as input, and use it to generate
* a camera-facing quad. Don't forget to pass the other important information (texture coordinates etc.) to the
* fragment shader.
* 2. Write the fragment shader. This should sample from the colour texture to get the flame colour based on the 
* particle's height (use the flameTime variable, which ranges in [0,1]). It should sample the alpha from the alpha
* texture, in a window of size texWindowSize which should scroll up the texture as the particle rises. Finally add
* a falloff based on the texture coordinates to get rid of the boxy square edges of the particles.
* 
* Note
* ====
* 
* For another example of a geometry shader, have a look at the first lab's LinearAlgebra project, and the DrawNormals shaders.
* You should be able to understand how these draw the normals for the meshes now!
*/

const int winWidth = 1280, winHeight = 720;

const Uint64 desiredFrametime = 33;

// --- Flame control parameters ---

int nFireParticles = 300; // No. of particles in the fire total.
float fireBaseRadius = 1.f; // Radius of the base of the fire. Determines initial positions of particles.

float flameDuration = 2.f; // How long a particle takes to reach the top of the flame
float flameHeight = 7.f; // Height of the flame
float flameFadeinEnd = 0.1f; // Up to this position in the flame, the particles linearly fade in (proportion, in [0,1]).
float flameFadeoutStart = 0.5f; // At this position in the flame the particles will linearly fade out (proportion, in [0,1]).

float flameTexWindowSize = 0.2f; // Determines the size of the windowed region of the alpha texture each
// flame particle samples from. Should be between 0 and 1.

// Parameters controlling curve of flame.
float flameBulgeHeight = 0.3f; // The height at which the flame is the widest (proportion, in [0,1])
float flameBulgeRadius = 1.3f; // The radius at the widest point (multiple of the initial radius at the base of the flame)
float flameTopRadius = 0.2f; // The radius the flame narrows to at the top (again multiple of initial radius)

// Solves for the coefficients of a cubic curve satisfying the flame parameters above.
Eigen::Vector4f solveForFlameCubic() {
	// Uses https://en.wikipedia.org/wiki/Polynomial_regression
	// to solve for a cubic that passes through the specified flame points.
	Eigen::Matrix<float, 3, 4> X;
	X <<
		1.0, 0.0, 0.0, 0.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, flameBulgeHeight, flameBulgeHeight* flameBulgeHeight, flameBulgeHeight* flameBulgeHeight* flameBulgeHeight;
	Eigen::Vector3f y(1.0, flameTopRadius, flameBulgeRadius);

	// Using pseudo-inverse here, as this isn't a square matrix.
	// This is generally a good idea anyway, as it's often more stable than .inverse()
	Eigen::Vector4f solution = (X.transpose() * X).completeOrthogonalDecomposition().pseudoInverse() * X.transpose() * y;
	return solution;
}

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

	// Prepare window1.f, 1.f, 1.f, 1.f
	SDL_Window* window = SDL_CreateWindow("Geometry Shader-Based Fire Exercise", 50, 50, winWidth, winHeight, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);

	GLenum result = glewInit();
	if (result != GLEW_OK) {
		throw std::runtime_error("GLEW couldn't initialize.");
	}

	gltInit();
	GLTtext* text = gltCreateText();
	gltSetText(text, (std::string("Animation demo")).c_str());
	glEnable(GL_MULTISAMPLE);

	{
		glhelper::ShaderProgram fireShader({ "../shaders/FireParticle.vert", "../shaders/FireParticle.geom", "../shaders/FireParticle.frag" });
		glhelper::ShaderProgram fireplaceShader({ "../shaders/TexturedMesh.vert", "../shaders/TexturedMesh.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);
		viewer.distance(20.f);

		glhelper::Mesh fire, fireplace;
		loadMesh(&fireplace, "../models/fireplace.obj");
		// Initialise fire vertices to random positions in a small circle
		// at the base of the fire.
		{
			std::vector<Eigen::Vector3f> fireInitParticlePositions(nFireParticles);
			std::vector<Eigen::Vector2f> fireRandSeedValues(nFireParticles);
			std::random_device dev;
			std::default_random_engine eng(dev());
			std::uniform_real_distribution<> dist(-fireBaseRadius, fireBaseRadius);
			std::uniform_real_distribution<> dist01(0.f, 1.f);
			// This uses rejection sampling
			// Generate random points in a square, then reject those that
			// don't fall within the desired circle.
			for (size_t i = 0; i < nFireParticles; ++i) {
				do
					fireInitParticlePositions[i] = Eigen::Vector3f(dist(eng), 0.f, dist(eng));
				while (fireInitParticlePositions[i].norm() > fireBaseRadius);

				fireRandSeedValues[i] = Eigen::Vector2f(dist01(eng), dist01(eng));
			}
			fire.vert(fireInitParticlePositions);
			fire.tex(fireRandSeedValues);
		}
		fire.shaderProgram(&fireShader);
		// Note we draw in GL_POINTS mode this time, as the input to our geometry shader will be 
		// a point cloud.
		// Note the draw mode is the input to the geometry shader, *not* its output (in this case, not
		// GL_TRIANGLES for example).
		fire.drawMode(GL_POINTS);
		fire.modelToWorld(makeTranslationMatrix(Eigen::Vector3f(0.f, -2.f, 0.f)));

		fireplace.shaderProgram(&fireplaceShader);
		fireplace.modelToWorld(makeTranslationMatrix(Eigen::Vector3f(0.f, -2.f, 0.f)) *
			makeScaleMatrix(4.0 * Eigen::Vector3f::Ones()));

		glProgramUniform1i(fireShader.get(), fireShader.uniformLoc("flameColorTex"), 0);
		glProgramUniform1i(fireShader.get(), fireShader.uniformLoc("flameAlphaTex"), 1);
		auto flameCoeffts = solveForFlameCubic();
		glProgramUniform4f(fireShader.get(), fireShader.uniformLoc("flameCubicCoeffts"), flameCoeffts[0], flameCoeffts[1], flameCoeffts[2], flameCoeffts[3]);
		glProgramUniform1f(fireShader.get(), fireShader.uniformLoc("duration"), flameDuration);
		glProgramUniform1f(fireShader.get(), fireShader.uniformLoc("flameHeight"), flameHeight);
		glProgramUniform1f(fireShader.get(), fireShader.uniformLoc("texWindowSize"), flameTexWindowSize);
		glProgramUniform1f(fireShader.get(), fireShader.uniformLoc("flameFadeoutStart"), flameFadeoutStart);
		glProgramUniform1f(fireShader.get(), fireShader.uniformLoc("flameFadeinEnd"), flameFadeinEnd);

		glProgramUniform1i(fireplaceShader.get(), fireplaceShader.uniformLoc("tex"), 0);

		// The colour texture is a 1D texture with a gradient specifying how flame particle colour varies with height.
		cv::Mat fireColorImage = cv::imread("../images/fire/flameColor.png");
		GLuint flameColorTexture;
		glGenTextures(1, &flameColorTexture);
		glBindTexture(GL_TEXTURE_1D, flameColorTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB8, fireColorImage.cols, 0, GL_BGR, GL_UNSIGNED_BYTE, fireColorImage.data);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_1D, 0);
		// The alpha texture contains squiggly lines. The particles sample in a window, scrolling up this texture
		// to create illusion of movement.
		cv::Mat fireAlphaImage = cv::imread("../images/fire/fireAlpha.png", cv::IMREAD_UNCHANGED);
		GLuint flameAlphaTexture;
		glGenTextures(1, &flameAlphaTexture);
		glBindTexture(GL_TEXTURE_2D, flameAlphaTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, fireAlphaImage.cols, fireAlphaImage.rows, 0, GL_RED, GL_UNSIGNED_BYTE, fireAlphaImage.data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		cv::Mat fireplaceTextureImage = cv::imread("../models/fireplace.jpg");
		GLuint fireplaceTexture;
		glGenTextures(1, &fireplaceTexture);
		glBindTexture(GL_TEXTURE_2D, fireplaceTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, fireplaceTextureImage.cols, fireplaceTextureImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, fireplaceTextureImage.data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);

		auto startTime = std::chrono::steady_clock::now();

		while (!shouldQuit) {
			Uint64 frameStartTime = SDL_GetTicks64();
			float animTimeSeconds = 1e-6f * (float)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - startTime).count();

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

				}

			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDisable(GL_BLEND);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fireplaceTexture);
			fireplace.render();
			glBindTexture(GL_TEXTURE_2D, 0);


			glProgramUniform1f(fireShader.get(), fireShader.uniformLoc("time"), animTimeSeconds);
			glProgramUniform1f(fireplaceShader.get(), fireplaceShader.uniformLoc("time"), animTimeSeconds);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_1D, flameColorTexture);
			glActiveTexture(GL_TEXTURE0+1);
			glBindTexture(GL_TEXTURE_2D, flameAlphaTexture);
			
			// Turn on blending so we blend the fire particles together nicely.
			// This time we use additive blending.
			// Using glBlendFunc like this means we'll add alpha times our new flame sample to the existing
			// colour in the buffer (rather than linearly combining like we have before).
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glBlendEquation(GL_FUNC_ADD);
			// We don't want to depth-test flame particles against one another, as we want their contributions
			// to add up. However we do want to depth test against other stuff in the scene so we don't draw the flame
			// on top of other objects.
			// Here I set glDepthMask to GL_FALSE meaning depth testing will take place but no depths will be drawn.
			// This, along with drawing the flame last should help depth testing work correctly.
			// In more complex scenes you may have to be more careful!
			glDepthMask(GL_FALSE);
			fire.render();
			glDepthMask(GL_TRUE);
			glBindTexture(GL_TEXTURE_1D, 0);
			glBindTexture(GL_TEXTURE_2D, 0);

			std::string textStr = std::string("Animation Time: ") + std::to_string(animTimeSeconds) + std::string(" seconds.");
			gltSetText(text, textStr.c_str());
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

		glDeleteTextures(1, &flameColorTexture);
		glDeleteTextures(1, &flameAlphaTexture);
		glDeleteTextures(1, &fireplaceTexture);
	}

	gltDeleteText(text);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

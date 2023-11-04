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
/* In this exercise you'll add a compute shader that updates the positions of particles in a simple particle system.
* You'll see that the scene is set up with Saturn at the centre and particles simulating its rings around it. If you
* press space a small moon will be dropped in near the rings.
* 
* Your goal is to add the compute shader that updates the positions of these particles according to gravitational forces
* from Saturn and the new moon.  You should implement the compute shader ParticlePhysics.comp to do this - this should
* work out forces on each particle based on the gravitational formula G m_1 m_2 r^-2. Use semi-implicit Euler to find
* the positions and normals (look back at the Physics notes if you need to).
* 
* Finally you'll need to add the compute shader call to the code below, and also add synchronisation to ensure the 
* compute shader is finished before the draw call to show the particles.
* 
* Extra Tasks
* ===========
* 
* If you finish early, consider these extra tasks:
* 1. Add collision to your physics engine. To do this you'll probably want to pass in the radii of the planets, as well
* as their masses and positions. On collision you could delete the particle, by marking it as inactive in some way and 
* changing BillboardParticle.geom to discard inactive particles.
* 2. Add more masses to the simulation (more moons/planets). The current system can handle up to 10, but you can of course
* change this. 
* 3. Also consider simulating the motion of the planets using a similar physics system. You could do this on the CPU and 
* pass the results to the compute shader. You can assume the mass of the rings is insignificant compared to the planets 
* and moons, so doesn't affect their orbits.
*/

const int winWidth = 1280, winHeight = 720;

const Uint64 desiredFrametime = 33;

float ringMinRadius = 5.f;
float ringMaxRadius = 6.f;
float particleInitialVelocity = 0.5f;
float particleMass = 0.1f;

float gravitationalConstant = 1e-2f;

float ringParticleSize = 0.03f;

int nParticles = 5000;

const int MAX_N_MASSES = 10;

bool ceresActive = false;


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
		glhelper::ShaderProgram texturedMeshShader({ "../shaders/TexturedMesh.vert", "../shaders/TexturedMesh.frag" });
		glhelper::ShaderProgram billboardParticleShader({ "../shaders/BillboardParticle.vert", "../shaders/BillboardParticle.geom", "../shaders/BillboardParticle.frag" });
		glhelper::ShaderProgram particlePhysicsShader({ "../shaders/ParticlePhysics.comp" });
		glhelper::RotateViewer viewer(winWidth, winHeight);
		viewer.distance(20.f);

		glhelper::Mesh sphereMesh;
		loadMesh(&sphereMesh, "../models/sphere.obj");

		GLuint ringVao;
		glGenVertexArrays(1, &ringVao);
		glhelper::ShaderStorageBuffer particleBuffer(nParticles * 4 * sizeof(float)),
			velocityBuffer(nParticles * 4 * sizeof(float));

		// Initialise ring particle positions and velocities		
		{
			std::vector<Eigen::Vector4f> particlePositions(nParticles), particleVelocities(nParticles);
			std::default_random_engine eng;
			std::uniform_real_distribution<> radDist(ringMinRadius, ringMaxRadius), angleDist(0.0f, 2.0f * (float)M_PI);

			for (size_t i = 0; i < nParticles; ++i) {
				float angle = angleDist(eng);
				float radius = radDist(eng);

				particlePositions[i] = radius * Eigen::Vector4f(sinf(angle), 0.f, cosf(angle), 1.0f);
				particleVelocities[i] = particleInitialVelocity * Eigen::Vector4f(1.f, 0.f, 0.f, 0.f);
				Eigen::Vector3f vel = -particlePositions[i].block<3,1>(0,0).normalized().cross(Eigen::Vector3f(0.f, 1.f, 0.f)) * particleInitialVelocity;
				particleVelocities[i].block<3, 1>(0, 0) = vel;
			}

			particleBuffer.update(particlePositions);
			velocityBuffer.update(particleVelocities);
		}

		glBindVertexArray(ringVao);
		glBindBuffer(GL_ARRAY_BUFFER, particleBuffer.get());
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		std::array<Eigen::Vector3f, MAX_N_MASSES> massLocations;
		std::array<float, MAX_N_MASSES> masses;
		int nMasses = 1;

		massLocations[0] = Eigen::Vector3f::Zero();
		masses[0] = 100.f;
		
		Eigen::Matrix4f saturnModelToWorld = Eigen::Matrix4f::Identity();
		Eigen::Matrix4f ceresModelToWorld = makeTranslationMatrix(Eigen::Vector3f(8.f, 0.f, 0.f)) * makeScaleMatrix(Eigen::Vector3f::Ones() * 0.1f);

		massLocations[1] = Eigen::Vector3f(8.f, 0.f, 0.f);
		masses[1] = 20.f;
		

		// Note we draw in GL_POINTS mode this time, as the input to our geometry shader will be 
		// a point cloud.
		// Note the draw mode is the input to the geometry shader, *not* its output (in this case, not
		// GL_TRIANGLES for example).
		sphereMesh.shaderProgram(&texturedMeshShader);

		glProgramUniform1f(billboardParticleShader.get(), billboardParticleShader.uniformLoc("particleSize"), ringParticleSize);
		glProgramUniform3f(billboardParticleShader.get(), billboardParticleShader.uniformLoc("particleColor"), 0.6f, 0.2f, 0.1f);

		glProgramUniform3fv(particlePhysicsShader.get(), particlePhysicsShader.uniformLoc("massPositions"), MAX_N_MASSES, massLocations[0].data());
		glProgramUniform1fv(particlePhysicsShader.get(), particlePhysicsShader.uniformLoc("masses"), MAX_N_MASSES, &(masses[0]));
		glProgramUniform1i(particlePhysicsShader.get(), particlePhysicsShader.uniformLoc("nMasses"), nMasses);
		glProgramUniform1f(particlePhysicsShader.get(), particlePhysicsShader.uniformLoc("gravitationalConstant"), gravitationalConstant);
		glProgramUniform1f(particlePhysicsShader.get(), particlePhysicsShader.uniformLoc("timeStep"), 1.f / 33.3f);
		glProgramUniform1f(particlePhysicsShader.get(), particlePhysicsShader.uniformLoc("particleMass"), particleMass);


		glProgramUniform1i(texturedMeshShader.get(), texturedMeshShader.uniformLoc("tex"), 0);

		GLuint saturnTexture, ceresTexture;

		{
			cv::Mat saturnTextureImage = cv::imread("../images/2k_saturn.jpg");
			glGenTextures(1, &saturnTexture);
			glBindTexture(GL_TEXTURE_2D, saturnTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, saturnTextureImage.cols, saturnTextureImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, saturnTextureImage.data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);

			cv::Mat ceresTextureImage = cv::imread("../images/2k_ceres_fictional.jpg");
			glGenTextures(1, &ceresTexture);
			glBindTexture(GL_TEXTURE_2D, ceresTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, ceresTextureImage.cols, ceresTextureImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, ceresTextureImage.data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

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
					if (event.key.keysym.sym == SDLK_SPACE) {
						ceresActive = true;
						nMasses = 2;
					}

				}

			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Your code here
			// Set up and dispatch your compute call here.
			// You should bind your particle position and velocity buffers
			// Normally you'd use glBindBufferBase for this, but the buffer class has
			// a bindBase method that does this. 
			// Use the compute shader (again, normally glUseProgram, but the class has use() 
			// and unuse() methods).
			// Dispatch with glDispatchCompute (set dimensions appropriately).

			// Your code here
			// Handle synchronisation. Add a glMemoryBarrier somewhere below here.
			// Pick an appropriate barrier to use (we're dealing with vertex attribute data).
			// Insert the barrier call in an appropriate place (really as late as possible, but make sure to do it
			// before the particle draw call)
			glDisable(GL_BLEND);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, saturnTexture);
			sphereMesh.modelToWorld(saturnModelToWorld);
			sphereMesh.render();
			glBindTexture(GL_TEXTURE_2D, 0);

			if (ceresActive) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, ceresTexture);
				sphereMesh.modelToWorld(ceresModelToWorld);
				sphereMesh.render();
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			glEnable(GL_BLEND);
			glDepthMask(GL_FALSE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBindVertexArray(ringVao);
			billboardParticleShader.use();
			glDrawArrays(GL_POINTS, 0, nParticles);
			billboardParticleShader.unuse();
			glDepthMask(GL_TRUE);

			glProgramUniform3fv(particlePhysicsShader.get(), particlePhysicsShader.uniformLoc("massPositions"), MAX_N_MASSES, massLocations[0].data());
			glProgramUniform1fv(particlePhysicsShader.get(), particlePhysicsShader.uniformLoc("masses"), MAX_N_MASSES, &(masses[0]));
			glProgramUniform1i(particlePhysicsShader.get(), particlePhysicsShader.uniformLoc("nMasses"), nMasses);

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

		glDeleteTextures(1, &saturnTexture);
	}

	gltDeleteText(text);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

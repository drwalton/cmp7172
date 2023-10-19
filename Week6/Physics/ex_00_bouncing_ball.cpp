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

#include <bullet/btBulletDynamicsCommon.h>

#include <opencv2/opencv.hpp>
#define GLT_IMPLEMENTATION
#include <gltext.h>
/* In this exercise you'll set up a simple simulation with a sphere bouncing on a solid fixed cuboid.
* You should set up the parameters of the sphere and cuboid to match the dimensions and initial positions
* of the objects rendered in the scene.
* Run the simulation each frame, and update the GL sphere to follow its transform given by the Bullet
* physics simulation. Make the ground plane static (and don't worry about its transform, it won't move!)
* To get the bouncing working properly, you'll need to set up correct parameters for gravity, restitution
* of the ground plane & sphere, and mass (for the sphere). For the ground remember setting mass to 1 will
* fix the rigidbody statically in place!
*/

const int winWidth = 1280, winHeight = 720;

const Uint64 desiredFrametime = 33;

float groundWidth = 10.f;
float groundHeight = 5.f;
float groundYPos = -5.f;

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
	gltSetText(text, (std::string("Physics demo")).c_str());
	glEnable(GL_MULTISAMPLE);

	{
		// This sets up the physics world for your simulation.
		// I used unique_ptr here for convenience but it would probably be neater
		// to wrap up the creation and destruction of the physics world into a class.
		std::unique_ptr<btDefaultCollisionConfiguration> collisionConfig = 
			std::make_unique<btDefaultCollisionConfiguration>();
		std::unique_ptr<btCollisionDispatcher> dispatcher = 
			std::make_unique<btCollisionDispatcher>(collisionConfig.get());
		std::unique_ptr<btBroadphaseInterface> overlappingPairCache = 
			std::make_unique<btDbvtBroadphase>();
		std::unique_ptr<btSequentialImpulseConstraintSolver> solver = 
			std::make_unique<btSequentialImpulseConstraintSolver>();
		std::unique_ptr<btDiscreteDynamicsWorld> world = 
			std::make_unique<btDiscreteDynamicsWorld>(
				dispatcher.get(), overlappingPairCache.get(), solver.get(), collisionConfig.get());

		// You may want to set appropriate gravity here.

		// When you create your shapes (box and sphere) add them to this array. 
		// You should then delete them at the end of the program.
		btAlignedObjectArray<btCollisionShape*> collisionShapes;
		{
			// Make a rigidbody for the floor
			// Make a box collision shape, set its transform, mass, inertia and restitution
			// then make the rigidbody with these properties and add it to the world.
		}

		{
			// Make a rigidbody for the ball
			// Make a sphere collision shape, set its transform, mass, inertia and restitution
			// then make the rigidbody with these properties and add it to the world.
			// I recommend setting body->setActivationState(DISABLE_DEACTIVATION)
			// By default, the sphere will be dectivated if it stops moving and you'll need to call
			// body->activate(); again for impulses and forces to have any effect.
			// This is more efficient, but annoying for debugging!
		}

		glhelper::ShaderProgram lambertianShader({ "../shaders/FixedColorLambertian.vert", "../shaders/FixedColorLambertian.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);
		viewer.distance(20.f);
		//glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh ballMesh, groundMesh;

        loadMesh(&ballMesh, "../models/sphere.obj");
        loadMesh(&groundMesh, "../models/cube.obj");
		ballMesh.shaderProgram(&lambertianShader);
		groundMesh.shaderProgram(&lambertianShader);

		auto groundTranslate = makeTranslationMatrix(Eigen::Vector3f(0.f, groundYPos, 0.f));
		auto groundScale = makeScaleMatrix(Eigen::Vector3f(groundWidth, groundHeight, groundWidth) * 0.5f);
		groundMesh.modelToWorld(groundTranslate * groundScale);

		ballMesh.modelToWorld(makeTranslationMatrix(Eigen::Vector3f(0.f, 5.f, 0.f)));

		glProgramUniform4f(lambertianShader.get(), lambertianShader.uniformLoc("color"), 1.f, 1.f, 1.f, 1.f);
		glProgramUniform3f(lambertianShader.get(), lambertianShader.uniformLoc("lightPosWorld"), 10.f, 10.f, 10.f);

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		while (!shouldQuit) {
			Uint64 frameStartTime = SDL_GetTicks64();

			// Add a command to step the simulation here.
			// Use desiredFrametime - this is in milliseconds so make sure to convert
			// to seconds for Bullet. Use maxSubSteps of 10. This means if the rendering
			// runs slower than 60FPS (which it will, it's set to 30FPS!) then Bullet can
			// perform up to 10 simulation steps using its fixed update rate of 60Hz.
			// This makes it much more stable!

			// Now get the transform of your sphere collisionObject and use it to
			// update your OpenGL sphere's modelToWorld property.
			// You can keep a reference to the sphere when you create it, or 
			// get it from the  world using world->getCollisionObjectArray() and selecting its
			// index (it'll be 1 if you added it second).

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
					// Set up some keybindings here to add impulses or forces to your sphere.
					// Experiment a bit with both! For forces you should hold down the key for
					// the force to accelerate the ball.
					// If you didn't set DISABLE_DEACTIVATION you'll want to call body->activate() before
					// applying forces & impulses.

				}

			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDisable(GL_CULL_FACE);
			glProgramUniform4f(lambertianShader.get(), lambertianShader.uniformLoc("color"), 0.1f, 0.8f, 0.8f, 1.f);
			ballMesh.render();
			glProgramUniform4f(lambertianShader.get(), lambertianShader.uniformLoc("color"), 0.f, 1.0f, 0.1f, 1.f);
			groundMesh.render();

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

		// Iterate through collisionShapes here and delete them.
		// We don't need to worry about the other properties, as std::unique_ptr
		// should automatically delete them.
	}

	gltDeleteText(text);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

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
/* In this exercise you'll try out adding forces and impulses to a rigidbody.
* We have a simple scene here with a planet and a moon (you may have to use your
* imagination a bit!)
* 
* You should add a gravitational force to the moon each frame to pull it towards 
* the planet. Also, give the moon an impulse at the start of the simulation to
* give it some initial velocity. This should allow it to orbit the planet.
* For gravitation, the formula is G*m_1*m_2 / (r*r) 
* Use planetMass and moonMass for this calculation.
* 
* Normally G should be the gravitational constant of about 6.7e-11
* Since everything here is on a small scale and we want plenty of gravity set
* it to about 5e-2 (you can play with this value if you'd like).
* 
* Quick tip on orbits - if your moon shoots off into space, your initial impulse
* was too fast (you've exceeded the escape velocity of the planet). Tone down
* your initial impulse. If it crashes into (and probably bounces off) the planet
* add more oomph to your impulse.
* 
* If you fancy try moving the start position and changing the impulse to get other
* orbits (can you set up a polar orbit?)
*/

const int winWidth = 1280, winHeight = 720;

const Uint64 desiredFrametime = 33;

float planetMass = 100.f;
float moonMass = 1.f;


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

void addSphereShape(btAlignedObjectArray<btCollisionShape*>& shapes, btDiscreteDynamicsWorld* world, const Eigen::Vector3f &position, float radius, float mass) {
	btCollisionShape* shape = new btSphereShape(radius);
	shapes.push_back(shape);
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(position.x(), position.y(), position.z()));
	btVector3 localInertia(0.f, 0.f, 0.f);
	if (mass >= 0.f) {
		shape->calculateLocalInertia(mass, localInertia);
	}
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
	rbInfo.m_restitution = 0.5f;
	btRigidBody* body = new btRigidBody(rbInfo);
	world->addRigidBody(body);
	body->setActivationState(DISABLE_DEACTIVATION);

}

Eigen::Matrix4f getRigidBodyTransform(btDynamicsWorld* world, int idx) {
	btCollisionObject* obj = world->getCollisionObjectArray()[idx];
	btRigidBody* body = btRigidBody::upcast(obj);
	btTransform trans;
	if (body && body->getMotionState()) {
		body->getMotionState()->getWorldTransform(trans);
	}
	else {
		trans = obj->getWorldTransform();
	}
	Eigen::Quaternionf rot(trans.getRotation().w(), trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z());
	Eigen::Matrix4f rotMat = Eigen::Matrix4f::Identity();
	rotMat.block<3, 3>(0, 0) = rot.matrix();
	return makeTranslationMatrix(Eigen::Vector3f(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z())) * rotMat;
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
		// No gravity this time! You'll add it yourself :)
		world->setGravity(btVector3(0, 0, 0));

		btAlignedObjectArray<btCollisionShape*> collisionShapes;

		addSphereShape(collisionShapes, world.get(), Eigen::Vector3f(0.f, 0.f, 0.f), 1.0f, 0.f);
		addSphereShape(collisionShapes, world.get(), Eigen::Vector3f(3.f, 0.f, 0.f), 0.1f, moonMass);

		btCollisionObject* planetObj = world->getCollisionObjectArray()[0];
		btRigidBody* planet = btRigidBody::upcast(planetObj);

		btCollisionObject* moonObj = world->getCollisionObjectArray()[1];
		btRigidBody* moon = btRigidBody::upcast(moonObj);

		glhelper::ShaderProgram lambertianShader({ "../shaders/FixedColorLambertian.vert", "../shaders/FixedColorLambertian.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);
		viewer.distance(20.f);
		//glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh sphereMesh;
        loadMesh(&sphereMesh, "../models/sphere.obj");
		sphereMesh.shaderProgram(&lambertianShader);

		Eigen::Matrix4f planetScale = makeScaleMatrix(Eigen::Vector3f(1.f, 1.f, 1.f));
		Eigen::Matrix4f moonScale = makeScaleMatrix(Eigen::Vector3f(1.f, 1.f, 1.f) * 0.1f);

		
		glProgramUniform4f(lambertianShader.get(), lambertianShader.uniformLoc("color"), 1.f, 1.f, 1.f, 1.f);
		glProgramUniform3f(lambertianShader.get(), lambertianShader.uniformLoc("lightPosWorld"), 10.f, 10.f, 10.f);

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Add an impulse to your moon here to get it into orbit!
		
		while (!shouldQuit) {
			Uint64 frameStartTime = SDL_GetTicks64();

			world->stepSimulation(desiredFrametime / 1000.f, 10);

			// Calculate gravity here.
			// Find the position of your moon and planet using getWorldTransform
			// Use this to get a direction vector for your gravity force.
			// To get the magnitude use the gravity formula above.
			// If you add this first, you can check the moon falls in towards the planet.
			// It should accelerate faster and faster as it goes, due to the inverse
			// square law.

			// Optional extra task - make the planet a dynamic object (set its mass to planetMass)
			// and apply the gravitational force to it too. The moon and planet should now rotate
			// around their common centre of mass. Does the planet orbit as well?




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

			glDisable(GL_CULL_FACE);
			glProgramUniform4f(lambertianShader.get(), lambertianShader.uniformLoc("color"), 0.f, 1.0f, 0.1f, 1.f);
			sphereMesh.modelToWorld(getRigidBodyTransform(world.get(), 0) * planetScale);
			sphereMesh.render();
			glProgramUniform4f(lambertianShader.get(), lambertianShader.uniformLoc("color"), 0.8f, 0.2f, 0.2f, 1.f);
			sphereMesh.modelToWorld(getRigidBodyTransform(world.get(), 1) * moonScale);
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
		for (int i = 0; i < collisionShapes.size(); ++i) {
			delete collisionShapes[i];
		}
	}

	gltDeleteText(text);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

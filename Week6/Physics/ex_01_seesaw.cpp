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
/* This exercise starts off with a simple scene made of boxes in Bullet, with two cubes of different masses on a seesaw
* Currently the boxes are all loose and there's no hinge joint fixing the seesaw together!
* Try adding a hinge constraint to this scene to fix the pivot and only let it rotate around the z axis.
* Once you have your hinge working properly, try changing the masses of the spheres and see if the seesaw tilts the correct
* way as expected.
*/

const int winWidth = 1280, winHeight = 720;

const Uint64 desiredFrametime = 33;

float groundWidth = 20.f;
float groundHeight = 5.f;
float groundYPos = -5.f;

float pivotHeight = 1.f;
float pivotWidth = 0.2f;
float pivotDepth = 2.f;

float pivotYPos = groundYPos + groundHeight * 0.5f + pivotHeight * 0.5f;

float seesawWidth = 8.f;
float seesawHeight = 0.5f;
float seesawYPos = groundYPos + groundHeight * 0.5f + pivotHeight + seesawHeight * 0.5f + 0.1f;

float cubeWidth = 1.8f;

// Change the cube masses here.
float cube1Mass = 3.f;
float cube2Mass = 2.f;

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

void addBoxShape(btAlignedObjectArray<btCollisionShape*>& shapes, btDiscreteDynamicsWorld* world, const Eigen::Vector3f &position, const Eigen::Vector3f &size, float mass) {
	btCollisionShape* shape = new btBoxShape(btVector3(size.x() * 0.5f, size.y() * 0.5f, size.z() * 0.5f));
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
		world->setGravity(btVector3(0, -10.0, 0));

		btAlignedObjectArray<btCollisionShape*> collisionShapes;

		addBoxShape(collisionShapes, world.get(), Eigen::Vector3f(0.f, groundYPos, 0.f), Eigen::Vector3f(groundWidth, groundHeight, groundWidth), 0.f);
		addBoxShape(collisionShapes, world.get(), Eigen::Vector3f(0.f, pivotYPos, 0.f), Eigen::Vector3f(pivotWidth, pivotHeight, pivotDepth), 0.f);
		addBoxShape(collisionShapes, world.get(), Eigen::Vector3f(0.f, seesawYPos, 0.f), Eigen::Vector3f(seesawWidth, seesawHeight, pivotDepth), 0.5f);
		addBoxShape(collisionShapes, world.get(), Eigen::Vector3f(-seesawWidth/2 + cubeWidth/2, seesawYPos + 5.f, 0.f), Eigen::Vector3f(cubeWidth, cubeWidth, cubeWidth), cube1Mass);
		addBoxShape(collisionShapes, world.get(), Eigen::Vector3f(seesawWidth/2 - cubeWidth/2, seesawYPos + 5.f, 0.f), Eigen::Vector3f(cubeWidth, cubeWidth, cubeWidth), cube2Mass);


		btCollisionObject* seesawObj = world->getCollisionObjectArray()[2];
		btRigidBody* seesaw = btRigidBody::upcast(seesawObj);
		// Add your hinge constraint here!
		// It should limit the rotation of the seesaw to just be around the z axis.
		// Note you can also set a 3D point inside the object which determines where
		// the joint pivots around.
		// You might also need to set the limits for the hinge, so it can rotate freely.

		glhelper::ShaderProgram lambertianShader({ "../shaders/FixedColorLambertian.vert", "../shaders/FixedColorLambertian.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);
		viewer.distance(20.f);
		//glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh cubeMesh;

        loadMesh(&cubeMesh, "../models/cube.obj");
		cubeMesh.shaderProgram(&lambertianShader);

		auto groundTranslate = makeTranslationMatrix(Eigen::Vector3f(0.f, groundYPos, 0.f));
		auto groundScale = makeScaleMatrix(Eigen::Vector3f(groundWidth, groundHeight, groundWidth) * 0.5f);
		auto groundModelToWorld = groundTranslate * groundScale;

		auto pivotTranslate = makeTranslationMatrix(Eigen::Vector3f(0.f, pivotYPos, 0.f));
		auto pivotScale = makeScaleMatrix(Eigen::Vector3f(pivotWidth, pivotHeight, pivotDepth) * 0.5f);
		auto pivotModelToWorld = pivotTranslate * pivotScale;

		auto seesawScale = makeScaleMatrix(Eigen::Vector3f(seesawWidth, seesawHeight, pivotDepth) * 0.5f);

		auto cubeScale = makeScaleMatrix(Eigen::Vector3f(cubeWidth, cubeWidth, cubeWidth) * 0.5f);

		glProgramUniform4f(lambertianShader.get(), lambertianShader.uniformLoc("color"), 1.f, 1.f, 1.f, 1.f);
		glProgramUniform3f(lambertianShader.get(), lambertianShader.uniformLoc("lightPosWorld"), 10.f, 10.f, 10.f);

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		while (!shouldQuit) {
			Uint64 frameStartTime = SDL_GetTicks64();

			world->stepSimulation(desiredFrametime / 1000.f, 10);


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
			cubeMesh.modelToWorld(groundModelToWorld);
			cubeMesh.render();
			glProgramUniform4f(lambertianShader.get(), lambertianShader.uniformLoc("color"), 0.8f, 0.2f, 0.2f, 1.f);
			cubeMesh.modelToWorld(pivotModelToWorld);
			cubeMesh.render();
			glProgramUniform4f(lambertianShader.get(), lambertianShader.uniformLoc("color"), 0.8f, 0.2f, 0.2f, 1.f);
			cubeMesh.modelToWorld(getRigidBodyTransform(world.get(), 2) * seesawScale);
			cubeMesh.render();
			glProgramUniform4f(lambertianShader.get(), lambertianShader.uniformLoc("color"), 0.f, 0.8f, 0.8f, 1.f);
			cubeMesh.modelToWorld(getRigidBodyTransform(world.get(), 3) * cubeScale);
			cubeMesh.render();
			glProgramUniform4f(lambertianShader.get(), lambertianShader.uniformLoc("color"), 0.f, 0.8f, 0.8f, 1.f);
			cubeMesh.modelToWorld(getRigidBodyTransform(world.get(), 4) * cubeScale);
			cubeMesh.render();

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

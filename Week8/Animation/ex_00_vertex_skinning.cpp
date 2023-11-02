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
/* In this exercise you'll implement vertex skinning, and use it to open and close the mouth of this bird model.
* This will help you get started with the first steps of setting up the full animated mesh. In the following project 
* you'll build on this further to load and use the animation included in the .gltf file.
* 
* To keep things simple for this lab, we've selected a file that contains a single mesh involved in this animation. 
* When working on your coursework, bear in mind that animations can involve multiple meshes, and part of the animation can
* involve transforming the meshes relative to one another. Common examples include eyes, hair and clothes of characters (these
* are often stored as separate meshes to simplify animation and allow them to be interchanged).
* 
* Your main objectives are as follows:
* 1. Modify loadMesh to grab the information you need to apply vertex skinning when rendering the mesh.
*    this information includes:
*	a. Bone names and offset matrices
*   b. Vertex weights
	Make sure to also set the mesh's boneIndices() and boneWeights() vertex buffers.
* 2. Implement AnimatedMesh.vert. This vertex shader should perform linear vertex blending according to the weights
*    you've loaded from the file.
*
* Once you've done this, you should see that the chick's beak opens and closes over time. This is controlled by the code at line 219 - 
* note this only adds a rotation to a single child bone and doesn't handle the full bone hierarchy. You'll do this in the next project!
* 
* Hints
* =====
* 
* + When loading offset matrices from Assimp, they're stored in column-major order, but both Eigen and GL use row-major order. You'll want
*    to transpose the matrices after loading from Assimp to account for this.
* + You can use the BoneInfo struct below to store info about your bones.
* + Assimp stores weights per-bone, but for vertex skinning you want weights per-vertex. That is, for each vertex you want a list
*    of the indices of the bones that affect it, and their weights. You'll need to adapt the info from Assimp to get this.
* + To open and close the mouth you could use a function based on sin. I used the following matrix:
*    Eigen::Matrix4f rotate = Eigen::Matrix4f::Identity();
*    rotate.block<3, 3>(0, 0) = Eigen::AngleAxisf(M_PI_2 * 0.25f * (1.f + sinf(animTimeSeconds)), Eigen::Vector3f(0.f, 0.f, -1.f)).matrix();
*    Here animTimeSeconds is a measure of elapsed time in seconds.
*/

const int winWidth = 1280, winHeight = 720;

const Uint64 desiredFrametime = 33;

// This is the maximum number of bones that can influence a single
// vertex. It's not the maximum total bone count.
const int MAX_BONES_PER_VERT = 4;

// This is the maximum number of bones possible in the whole animated
// mesh.
const int MAX_BONES = 40;

struct BoneInfo {
	std::string name;
	Eigen::Matrix4f offsetMatrix;
};

void loadMesh(glhelper::Mesh* mesh, const std::string &filename, std::vector<BoneInfo> &boneInfo) 
{
	Assimp::Importer importer;
	importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	const aiScene* aiscene = importer.GetScene();

	std::cout << "Scene info\n" <<
		"==========\n" <<
		"Num meshes: " << aiscene->mNumMeshes << 
		"\nNum animations: " << aiscene->mNumAnimations <<
		"\nNum textures: " << aiscene->mNumTextures <<
		"\nNum materials: " << aiscene->mNumMaterials <<
		std::endl;

	const aiMesh* aimesh = aiscene->mMeshes[0];

	// Your code here!
	// Load the bone information into the supplied boneInfo array.
	// Be careful when loading the bones' offset matrices!
	// Remember to transpose these due to the row-major/column-major thing.

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

	std::vector<Eigen::Vector4i> boneIndices(aimesh->mNumVertices);
	std::vector<Eigen::Vector4f> boneWeights(aimesh->mNumVertices);

	// Your code here!
	// Load the boneIndices and boneWeights from the assimp file.
	// I'm suggesting you use Vector4i and Vector4f - this means each vertex can only 
	// be affected by 4 bones (this is fine for this model, no vertex is affected by more
	// bones than this).
	// I suggest setting boneIndices to -1 initially - this can be your default index
	// used when less than 4 bones affect a vertex.

	mesh->vert(verts);
	mesh->norm(norms);
	mesh->elems(elems);
	mesh->tex(uvs);
	
	mesh->boneIndices(boneIndices);
	mesh->boneWeights(boneWeights);
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
	SDL_Window* window = SDL_CreateWindow("Animation Exercise", 50, 50, winWidth, winHeight, SDL_WINDOW_OPENGL);
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
		glhelper::ShaderProgram animatedMeshShader({ "../shaders/AnimatedMesh.vert", "../shaders/AnimatedMesh.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);
		viewer.distance(20.f);
		//glhelper::FlyViewer viewer(winWidth, winHeight);

		glhelper::Mesh chickMesh;
		std::vector<BoneInfo> chickBoneInfo;
        loadMesh(&chickMesh, "../models/animated_chick/scene.gltf", chickBoneInfo);
		chickMesh.shaderProgram(&animatedMeshShader);
		chickMesh.modelToWorld(makeTranslationMatrix(Eigen::Vector3f(0.f, -3.f, 0.f)));

		cv::Mat texImage = cv::imread("../models/animated_chick/textures/my_67_baseColor.png");
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texImage.cols, texImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, texImage.data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glProgramUniform1i(animatedMeshShader.get(), animatedMeshShader.uniformLoc("tex"), 0);
		glProgramUniform3f(animatedMeshShader.get(), animatedMeshShader.uniformLoc("lightPosWorld"), 10.f, 10.f, 10.f);

		std::array<Eigen::Matrix4f, MAX_BONES> boneMatrices;
		for (size_t i = 0; i < MAX_BONES; ++i) {
			boneMatrices[i] = Eigen::Matrix4f::Identity();
		}

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

			// The below code sets up all the bone Matrices for your code.
			// This is a very basic setup - it sets all of them to the identity except for one, which opens and closes the chick's beak.
			for (size_t i = 0; i < chickBoneInfo.size(); ++i) {
				Eigen::Matrix4f boneMatrix = chickBoneInfo[i].offsetMatrix;

				if (chickBoneInfo[i].name == std::string("Head_M 010")) {
					Eigen::Matrix4f rotate = Eigen::Matrix4f::Identity();
					rotate.block<3, 3>(0, 0) = Eigen::AngleAxisf(M_PI_2 * 0.25f * (1.f + sinf(animTimeSeconds)), Eigen::Vector3f(0.f, 0.f, -1.f)).matrix();
					boneMatrix = rotate * boneMatrix;
				}

				boneMatrices[i] = chickBoneInfo[i].offsetMatrix.inverse() * boneMatrix;
			}
			glProgramUniformMatrix4fv(animatedMeshShader.get(), animatedMeshShader.uniformLoc("boneMatrices"), MAX_BONES, GL_FALSE, boneMatrices[0].data());
			glDisable(GL_CULL_FACE);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			chickMesh.render();
			glBindTexture(GL_TEXTURE_2D, 0);

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

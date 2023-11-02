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
/* In this exercise you'll carry out the remaining steps to load a full animation!
* 
* First copy the relevant bits of code from the previous lab.
* 
* Your tasks here are to:
* 1. Load a more complete set of data for each bone, including its place in the hierarchy
*    (parent and child bones) and its animation data.
* 2. Use the loaded animation data to compute matrices for the bones in the skeleton each frame.
* 
* This, together with your previous code and the AnimatedMesh.vert you've already created,
* should be enough to get the animation of the chick moving around playing!
* 
* For this lab, our loaded file has only one mesh in the aiscene, and one animation. You're free to
* use this information to simplify your code. Do consider for your coursework how you might extend this
* to handle more complex scenarios. Animations in other files could involve other meshes, and move the
* meshes relative to one another. Think about how you might implement this (e.g. computing an animation
* for the full node tree, and using the results to update the skeletons for all meshes, and their individual
* modelToWorld matrices.
*/

const int winWidth = 1280, winHeight = 720;

const Uint64 desiredFrametime = 33;

// This is the maximum number of bones that can influence a single
// vertex. It's not the maximum total bone count.
const int MAX_BONES_PER_VERT = 4;

// This is the maximum number of bones possible in the whole animated
// mesh.
const int MAX_BONES = 40;

// This is the class I suggest you use to store the skeletal animation for a single bone in the mesh.
struct BoneAnimation {
	// The duration is the total length of the animation (for rotation, position and scale keys).
	float duration;

	// These arrays contain keys for rotation, position and scale, and times to display them.
	// Be careful - the number and timings of the keys may not match up
	// For example it's fine to have 10 rotation keys and 20 position keys, and for their
	// display times to be different!
	std::vector<Eigen::Quaternionf> rotationKeys;
	std::vector<float> rotationTimes;
	std::vector<Eigen::Vector3f> positionKeys;
	std::vector<float> positionTimes;
	std::vector<Eigen::Vector3f> scaleKeys;
	std::vector<float> scaleTimes;

	Eigen::Matrix4f evaluate(float time) {
		// Your code here 
		// Rather than the identity, return the matrix that scales, then rotates, then translates this bone.
		return Eigen::Matrix4f::Identity();
	}

	// In the event that no keyframes are present, all the functions below
	// should return identity matrices.
	// E.g. if rotationKeys.length() == 0
	// then findRotation should return the identity.

	Eigen::Matrix4f findPosition(float time) {
		// Your code here.
		// Linearly interpolate positionKeys based on positionTimes.
		return Eigen::Matrix4f::Identity();
	}

	Eigen::Matrix4f findRotation(float time) {
		// Your code here.
		// Linearly interpolate rotationKeys based on rotationTimes.
		// Convert the result to a 4x4 matrix
		return Eigen::Matrix4f::Identity();
	}

	Eigen::Matrix4f findScale(float time) {
		// Your code here.
		// Linearly interpolate scaleKeys based on scaleTimes.
		return Eigen::Matrix4f::Identity();
	}
};

// Here's the suggested struct to store info for a single bone in the mesh.
// We have some new information this time, the indices of the parent to this bone
// and to any children it might have. Set the parentIdx to -1 for any bones without parents
// (just the root bone).
// We've also added a BoneAnimation instance, to store the key positions, rotations and scales
// for this bone.
struct BoneInfo {
	std::string name;
	Eigen::Matrix4f offsetMatrix;
	int parentIdx;
	std::vector<int> childIndices;
	BoneAnimation animation;
};

void loadAnimatedMesh(glhelper::Mesh* mesh, const std::string &filename, std::vector<BoneInfo> &boneInfo) 
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
	// You can start from your code from the previous lab.
	// I'd recommend using a std::map like this to keep track of 
	// where in the boneInfo array a bone of a particular name has.
	std::map<std::string, int> boneNameToIdx;
	
	// Load the bone information (offset matrix). Also populate the 
	// boneNameToIdx map as you go along.
	// You'll populate the remaining info (parent and child indices, animation) later.

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
	std::vector<int> boneNumWeights(aimesh->mNumVertices);

	for (size_t v = 0; v < aimesh->mNumVertices; ++v) {
		boneIndices[v] = -Eigen::Vector4i::Ones();
		boneWeights[v] = Eigen::Vector4f::Zero();
		boneNumWeights[v] = 0;
	}

	// Your code here!
	// Load the boneIndices and boneWeights from the assimp file.
	// You can copy this part from the previous lab.

	// Your code here!
	// Fill out the parent and child information in the boneInfo array.
	// You should get the hierarchy information from the node information in aiScene.
	// Start from aiscene->mRootNode and work down from there.
	// Associate nodes with bones using their names (the boneNameToIdx map
	// should come in handy here!)

	// Your code here!
	// Load the animations into each bone of the mesh.
	// This file only contains one animation, so you can use aiscene->mAnimations[0].
	// Look through the "channels" of the animation - each corresponds to a bone.
	// Find which one by looking at the channel's mNodeName (and use your nice std::map again).
	// Load all the times and position, rotation and scale keys into the BoneInfo instances BoneAnimation class.
	// Note assimps animation times are given in milliseconds, so make sure to convert them to seconds.

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
        loadAnimatedMesh(&chickMesh, "../models/animated_chick/scene.gltf", chickBoneInfo);
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

			// Your code here !
			// Now let's use the animation!
			// we need to find the matrix for each bone and set the right value in the boneMatrices array.
			// Remember from the slides your ultimate goal here is to get the following for each bone:
			// 
			// offsetRoot.inverse() * animRoot * ..... * animParent * animThisBone * offsetThisBone
			//                        (-------------------------------------------)
			// Here,                   this bit ^ is the multiple of all the animation matrices from this bone,
			// working up through its parent bones to the root.
			// All animated matrices should be evaluated according to the current time animTimeSeconds.
			// 
			// I recommend working recursively
			// Start from the root bone(s) (ones with parentIdx -1)
			// Set their matrices to offsetMatrix.inverse() * animation
			// WHere animation comes from evaluating the animation at this bone.
			// Work down the hierarchy, set each node to its parent bone's matrix times 
			// the animation at this node.
			// Finally multiply each matrix in boneMatrices by its offset matrix to get the final results.

			for (size_t i = 0; i < chickBoneInfo.size(); ++i) {
				boneMatrices[i] =  boneMatrices[i] * chickBoneInfo[i].offsetMatrix;
			}
			glProgramUniformMatrix4fv(animatedMeshShader.get(), animatedMeshShader.uniformLoc("boneMatrices"), MAX_BONES, GL_FALSE, boneMatrices[0].data());
			glDisable(GL_CULL_FACE);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			chickMesh.render();
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
	}

	gltDeleteText(text);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

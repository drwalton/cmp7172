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
/* In this task you'll implement the Kajiya-Kay anisotropic shader, and
* use it to light hair in a more realistic way than isotropic models
* like Blinn-Phong.
* 
* An example of what your output might look like is included in images/kajiya_kay.png
* 
* You can press the space bar to make the light rotate to preview your 
* lighting. Press the H key to change the colour of the hair. Use the 
* e, i, w and m keys (with or without SHIFT) to increase/decrease the
* other shader parameters.
* 
* Tasks
* -----
* 
* 1. Adapt the LoadMesh program below to load the extra information you'll
* need to run the Kajiya-Kay shader. Note for these particular meshes, they've been
* set up so the bitangents lie along the direction of the hair strands.
* 2. Implement the shaders KajiyaKay.vert and KajiyaKay.frag. Use the settings already
* defined here to control the effect (specularExponent, specularIntensity).
* I suggest adding the simple self-shadowing term described in the lecture notes, and 
* controlling it with the selfShadowMin and selfShadowFadeoutWidth uniforms.
* 
* Hints
* -----
* 
* Remember to use the bitangents from Assimp - the tangents don't lie along the hair strands!
*/

const int winWidth = 1280, winHeight = 720;

const Uint64 desiredFrametime = 33;

float theta = 0.0f;
bool lightRotating = false;
Eigen::Vector3f lightPos(5.f*sinf(theta), 5.f, 5.f*cosf(theta));
float specularExponent = 30.f;
float specularIntensity = 0.5f;
float selfShadowMin = 0.f;
float selfShadowFadeoutWidth = 0.3f;

std::vector<Eigen::Vector3f> hairColors = {
	Eigen::Vector3f(1.0f, 1.0f, 1.0f),
	Eigen::Vector3f(0.3f, 0.3f, 0.3f),
	Eigen::Vector3f(0.8f, 0.8f, 0.2f),
	Eigen::Vector3f(0.5f, 0.4f, 0.2f),
	Eigen::Vector3f(0.2f, 0.75f, 0.75f),
	Eigen::Vector3f(1.0f, 0.5, 0.75f)
};
int hairColorIdx = 0;

void loadMesh(glhelper::Mesh* mesh, const std::string &filename, int idx = 0) 
{
	Assimp::Importer importer;
	// --- Your Code Here ---
	// Modify the line below to load the tangent information you need. 
	// Also add the relevant tangents to the mesh (remember it's the 
	// bitangents you're after!)
	importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	const aiScene* aiscene = importer.GetScene();
	const aiMesh* aimesh = aiscene->mMeshes[idx];

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

void setText(GLTtext* text) {
	std::stringstream textStream;
	textStream << "Specular exponent " << specularExponent << ", intensity " << specularIntensity <<
		"Self shadow min " << selfShadowMin << " fadeout width " << selfShadowFadeoutWidth;
	gltSetText(text, textStream.str().c_str());
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
	setText(text);
	glEnable(GL_MULTISAMPLE);

	{
		glhelper::ShaderProgram headShader({ "../shaders/LambertFixedColor.vert", "../shaders/LambertFixedColor.frag" });
		glhelper::ShaderProgram hairShader({ "../shaders/KajiyaKay.vert", "../shaders/KajiyaKay.frag" });
		glhelper::ShaderProgram fixedColorShader({ "../shaders/FixedColor.vert", "../shaders/FixedColor.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);
		//glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh hairMesh, headMesh, sphereMesh, hairySphereMesh;

        loadMesh(&sphereMesh, "../models/sphere.obj");
        loadMesh(&hairySphereMesh, "../models/sphere_hires.obj");
		loadMesh(&hairMesh, "../models/hair.obj");
		loadMesh(&headMesh, "../models/head.obj");
		hairMesh.shaderProgram(&hairShader);
		headMesh.shaderProgram(&headShader);
		hairySphereMesh.shaderProgram(&hairShader);
		headMesh.modelToWorld(makeTranslationMatrix(Eigen::Vector3f(-2.f, 0.f, 0.f)));
		hairMesh.modelToWorld(makeTranslationMatrix(Eigen::Vector3f(-2.f, 0.f, 0.f)));
		hairySphereMesh.modelToWorld(makeTranslationMatrix(Eigen::Vector3f(2.f, 0.f, 0.f)));
		sphereMesh.shaderProgram(&fixedColorShader);
		sphereMesh.modelToWorld(makeTranslationMatrix(lightPos));

		glProgramUniform4f(fixedColorShader.get(), fixedColorShader.uniformLoc("color"), 1.f, 1.f, 1.f, 1.f);
		glProgramUniform4f(headShader.get(), headShader.uniformLoc("color"), 0.3f, 0.3f, 0.3f, 1.f);
		glProgramUniform3f(hairShader.get(), hairShader.uniformLoc("hairColor"), hairColors[hairColorIdx].x(), hairColors[hairColorIdx].y(), hairColors[hairColorIdx].z());
		glProgramUniform3f(hairShader.get(), hairShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());
		glProgramUniform1i(hairShader.get(), hairShader.uniformLoc("texture"), 0);
		glProgramUniform1f(hairShader.get(), hairShader.uniformLoc("specularExponent"), specularExponent);
		glProgramUniform1f(hairShader.get(), hairShader.uniformLoc("specularIntensity"), specularIntensity);
		glProgramUniform1f(hairShader.get(), hairShader.uniformLoc("selfShadowMin"), selfShadowMin);
		glProgramUniform1f(hairShader.get(), hairShader.uniformLoc("selfShadowFadeoutWidth"), selfShadowFadeoutWidth);
		glProgramUniform3f(headShader.get(), headShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		GLuint hairTexture;
		{
			cv::Mat hairImage = cv::imread("../images/hair_white.jpg");
			glGenTextures(1, &hairTexture);
			glBindTexture(GL_TEXTURE_2D, hairTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
				hairImage.cols, hairImage.rows,
				0, GL_BGR, GL_UNSIGNED_BYTE, hairImage.data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glGenerateTextureMipmap(hairTexture);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

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
					else if (event.key.keysym.sym == SDLK_h) {
						++hairColorIdx;
						hairColorIdx %= hairColors.size();
						glProgramUniform3f(hairShader.get(), hairShader.uniformLoc("hairColor"), hairColors[hairColorIdx].x(), hairColors[hairColorIdx].y(), hairColors[hairColorIdx].z());
					}
					else if (event.key.keysym.sym == SDLK_e) {
						if (event.key.keysym.mod & KMOD_SHIFT) {
							specularExponent -= 1.f;
							specularExponent = std::max(specularExponent, 0.f);
						}
						else {
							specularExponent += 1.f;
						}
						glProgramUniform1f(hairShader.get(), hairShader.uniformLoc("specularExponent"), specularExponent);
						setText(text);
					}
					else if (event.key.keysym.sym == SDLK_i) {
						if (event.key.keysym.mod & KMOD_SHIFT) {
							specularIntensity -= 0.01f;
							specularIntensity = std::max(specularIntensity, 0.f);
						}
						else {
							specularIntensity += 0.01f;
						}
						glProgramUniform1f(hairShader.get(), hairShader.uniformLoc("specularIntensity"), specularIntensity);
						setText(text);
					}
					else if (event.key.keysym.sym == SDLK_m) {
						if (event.key.keysym.mod & KMOD_SHIFT) {
							selfShadowMin -= 0.01f;
						}
						else {
							selfShadowMin += 0.01f;
						}
						glProgramUniform1f(hairShader.get(), hairShader.uniformLoc("selfShadowMin"), selfShadowMin);
						setText(text);
					}
					else if (event.key.keysym.sym == SDLK_w) {
						if (event.key.keysym.mod & KMOD_SHIFT) {
							selfShadowFadeoutWidth -= 0.01f;
							selfShadowFadeoutWidth = std::max(selfShadowFadeoutWidth, 0.f);
						}
						else {
							selfShadowFadeoutWidth += 0.01f;
						}
						glProgramUniform1f(hairShader.get(), hairShader.uniformLoc("selfShadowFadeoutWidth"), selfShadowFadeoutWidth);
						setText(text);
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
				glProgramUniform3f(hairShader.get(), hairShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());
				glProgramUniform3f(headShader.get(), headShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());
			}

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, hairTexture);
			headMesh.render();
			hairMesh.render();
			hairySphereMesh.render();
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
		glDeleteTextures(1, &hairTexture);
	}

	gltDeleteText(text);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

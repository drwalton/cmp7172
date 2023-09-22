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

/* This existing application shows a mesh illuminated with Phong shading
* based on the mesh normals. Your goal here is to load the normal map texture from 
* images/normalmap.png and use it to modify the mesh normals and implement normal mapping
* 
* There are a few changes you'll need to make:
* 1. Modify the loadSpotMesh function so that Assimp generates tangents and bitangents, and
*    add them to the mesh.
* 2. Load the normal map texture, and link it to the shader.
* 3. Adapt the shader to implement normal mapping - change the normal used in
*    TexturedMeshNormal.frag based on the normal map texture
* Your final result should look something like the result in images/ex_02_normal_mapping.png
*
* Hints
* -----
* 1. Think about which space you want to transform your normals from and which space
*    they're mapping to. The lighting in the existing shader is computed in world space,
*    so it's probably easiest to find your perturbed normals in world space too.
*    This means the vectors in your TBN matrix should be in world space.
*/

const int winWidth = 1024, winHeight = 768;

const Uint64 desiredFrametime = 33;

float theta = 0.0f;
bool lightRotating = false;
Eigen::Vector3f lightPos(5.f * sinf(theta), 5.f, 5.f * cosf(theta));
float lightIntensity = 60.f;
float specularity = 10.f;

void loadSpotMesh(glhelper::Mesh* mesh) 
{
	// ----- Your code here -----
	// Change the code here to generate tangents and bitangents when loading the mesh, and add
	// them to the glhelper::Mesh instance.
	// You can use the Mesh::tangent() and Mesh::bitangent() methods to do this, they work
	// the same way as the Mesh::norm() method.

	Assimp::Importer importer;
	importer.ReadFile("../models/spot/spot_triangulated.obj", aiProcess_Triangulate | aiProcess_GenSmoothNormals);
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

Eigen::Matrix4f makeTranslationMatrix(const Eigen::Vector3f& translate)
{
	Eigen::Matrix4f matrix = Eigen::Matrix4f::Identity();
	matrix.block<3, 1>(0, 3) = translate;
	return matrix;
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

	{
		glhelper::ShaderProgram normalMapShader({ "../shaders/TexturedMeshNormal.vert", "../shaders/TexturedMeshNormal.frag" });
		glhelper::ShaderProgram fixedColorShader({ "../shaders/FixedColor.vert", "../shaders/FixedColor.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);
		//glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh spotMesh, sphereMesh;
		loadSpotMesh(&spotMesh);
		loadSphereMesh(&sphereMesh);

		spotMesh.shaderProgram(&normalMapShader);
		sphereMesh.shaderProgram(&fixedColorShader);

		lightPos << 5.f * sinf(theta), 5.f, 5.f * cosf(theta);
		sphereMesh.modelToWorld(makeTranslationMatrix(lightPos));
		glProgramUniform3f(normalMapShader.get(), normalMapShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());
		glProgramUniform1f(normalMapShader.get(), normalMapShader.uniformLoc("lightIntensity"), lightIntensity);
		glProgramUniform1f(normalMapShader.get(), normalMapShader.uniformLoc("specularity"), specularity);

		// --- Your code here ---
		// This currently only loads an albedo texture - adapt it to 
		// also load the normalmap.png texture and link it to the shader
		// (set the shader's sampler uniform to a suitable index, and bind the
		// texture to the same image unit).
		glProgramUniform1i(normalMapShader.get(), normalMapShader.uniformLoc("albedoTex"), 0);
		glProgramUniform4f(fixedColorShader.get(), fixedColorShader.uniformLoc("color"), 1.f, 1.f, 1.f, 1.f);

		GLuint albedoTexture;

		{
			cv::Mat image = cv::imread("../models/spot/spot_texture.png");
			cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
			albedoTexture = createTexture(image);
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
				viewer.processEvent(event);
				if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.sym == SDLK_SPACE) {
						lightRotating = !lightRotating;
					}
				}
			}

			if (lightRotating) {
				theta += 0.01f;
				if (theta > 2 * 3.14159f) theta = 0.f;
				lightPos << 5.f * sinf(theta), 5.f, 5.f * cosf(theta);
				sphereMesh.modelToWorld(makeTranslationMatrix(lightPos));
				glProgramUniform3f(normalMapShader.get(), normalMapShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());
			}

			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDisable(GL_CULL_FACE);
			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, albedoTexture);
			// --- Your code here ---
			// We've only bound the albedo texture so far - bind the 
			// normal texture too!

			spotMesh.render();
			sphereMesh.render();

			SDL_GL_SwapWindow(window);

			Uint64 elapsedFrameTime = SDL_GetTicks64() - frameStartTime;
			if (elapsedFrameTime < desiredFrametime) {
				SDL_Delay(desiredFrametime - elapsedFrameTime);
			}
		}

		glDeleteTextures(1, &albedoTexture);
		// --- Your code here ---
		// Remember to delete your normal texture once you're done with it too!
	}


	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

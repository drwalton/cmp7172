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
/* In this exercise you'll use a tessellation shader to render a heightmapped model of the moon.
* The approach will be a little different - first our initial control mesh is made of triangles, not quads.
* Secondly, we'll be displacing our vertices outward from the centre of a sphere according to the height texture, not 
* moving them along y.
* 
* Again you should implement the tessellation control and evaluation shaders.
* 1. The tessellation control shader should be similar - you have fewer variables to set this time however, as triangles have
* 3 outer tessellation levels and one inner tessellation level.
* 2. Your evaluation shader will be somewhat different. Recall gl_TessCoord for triangles is a 3D value, contaning barycentric
* coordinates for the tessellated vertex's position in the triangle.
* Use these coordinates to find the texture coordinate and position of the vertex through barycentric interpolation.
* Move the vertex in/out to be exactly "radius" from the centre (use the radius uniform).
* Set the normal to be the normalised position (note this only works because it's a sphere!)
* At this point, you should already see the low-poly sphere is becoming smooth.
* Now, sample from the height texture and use this to move the vertex out to add the geometric detail.
* 
* Hints
* =====
* In case you do this yourself, you need to be a bit careful about your sphere mesh. The default UV sphere from Blender
* causes issues as the vertices at the poles are not all colocated. You may want to join these vertices in the UV map
* to fix this (select the top or bottom row, and "merge at centre").
* 
* Note
* ====
* These shaders just assume normals for all vertices point outward from the sphere (they don't consider the heightmap).
* How could you modify this code to use more accurate normals?
* 
*/

const int winWidth = 1280, winHeight = 720;

const Uint64 desiredFrametime = 33;

const int meshWidth = 4;

float tessLevel = 2.f;
float heightScale = 0.1f;
float theta = -1.0f;
bool lightRotating = false;
Eigen::Vector3f lightDir = Eigen::Vector3f(1.f, 1.f, 0.f).normalized();

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

	// Prepare window
	SDL_Window* window = SDL_CreateWindow("Tesselation Shader-Based Heightfield Rendering Exercise", 50, 50, winWidth, winHeight, SDL_WINDOW_OPENGL);
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
		glhelper::ShaderProgram heightfieldShader({ 
			"../shaders/SphereDisplacement.vert", "../shaders/SphereDisplacement.tesc", 
			"../shaders/SphereDisplacement.tese", "../shaders/SphereDisplacement.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);

		glPatchParameteri(GL_PATCH_VERTICES, 3);

		glhelper::Mesh moonMesh;
		loadMesh(&moonMesh, "../models/lowpolysphere.obj");

		moonMesh.shaderProgram(&heightfieldShader);
		moonMesh.drawMode(GL_PATCHES);

		glProgramUniform1i(heightfieldShader.get(), heightfieldShader.uniformLoc("heightTexture"), 0);
		glProgramUniform1i(heightfieldShader.get(), heightfieldShader.uniformLoc("colorTexture"), 1);
		glProgramUniform1i(heightfieldShader.get(), heightfieldShader.uniformLoc("normalTexture"), 2);
		glProgramUniform1f(heightfieldShader.get(), heightfieldShader.uniformLoc("heightScale"), heightScale);
		glProgramUniform1f(heightfieldShader.get(), heightfieldShader.uniformLoc("radius"), 1.0f);
		lightDir << sinf(theta), 0.f, cosf(theta);
		glProgramUniform3f(heightfieldShader.get(), heightfieldShader.uniformLoc("lightDir"), lightDir.x(), lightDir.y(), lightDir.z());

		glProgramUniform1f(heightfieldShader.get(), heightfieldShader.uniformLoc("tessLevel"), tessLevel);
		gltSetText(text, (std::string("Tess level ") + std::to_string(tessLevel)).c_str());
		

		GLuint colorTexture;
		GLuint heightTexture;
		GLuint normalTexture;
		{
			cv::Mat depthImage = cv::imread("../images/moon_heightmap.jpg");
			cv::cvtColor(depthImage, depthImage, cv::COLOR_BGR2GRAY);
			glGenTextures(1, &heightTexture);
			glBindTexture(GL_TEXTURE_2D, heightTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, depthImage.cols, depthImage.rows, 0, GL_RED, GL_UNSIGNED_BYTE, depthImage.data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);

			cv::Mat colorImage = cv::imread("../images/moon_albedo.jpg");
			glGenTextures(1, &colorTexture);
			glBindTexture(GL_TEXTURE_2D, colorTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, colorImage.cols, colorImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, colorImage.data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);

			cv::Mat normalImage = cv::imread("../images/moon_normalmap.jpg");
			glGenTextures(1, &normalTexture);
			glBindTexture(GL_TEXTURE_2D, normalTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, colorImage.cols, colorImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, colorImage.data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);

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
					if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_UP) {
						if (event.key.keysym.sym == SDLK_DOWN) {
							if (event.key.keysym.mod & KMOD_LSHIFT) {
								heightScale -= 0.01f;
								if (heightScale < 0.f) heightScale = 0.f;
							}
							else {
								tessLevel -= 1;
								tessLevel = std::max(tessLevel, 1.f);
							}
						}
						if (event.key.keysym.sym == SDLK_UP) {
							if (event.key.keysym.mod & KMOD_LSHIFT) {
								heightScale += 0.01f;
							}
							else {
								tessLevel += 1;
							}
						}
						glProgramUniform1f(heightfieldShader.get(), heightfieldShader.uniformLoc("heightScale"), heightScale);
						glProgramUniform1f(heightfieldShader.get(), heightfieldShader.uniformLoc("tessLevel"), tessLevel);
					}
					gltSetText(text, (std::string("Tess level ") + std::to_string(tessLevel) + std::string(" Height scale ") + std::to_string(heightScale)).c_str());
					if (event.key.keysym.sym == SDLK_SPACE) {
						lightRotating = !lightRotating;
					}
				}

			}
			if (lightRotating) {
				theta += 0.01f;
				if (theta > 2 * 3.14159f) theta = 0.f;
				lightDir << sinf(theta), 0.f, cosf(theta);
				glProgramUniform3f(heightfieldShader.get(), heightfieldShader.uniformLoc("lightDir"), lightDir.x(), lightDir.y(), lightDir.z());
			}

			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, heightTexture);
			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, colorTexture);
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, normalTexture);
			moonMesh.render();
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

		glDeleteTextures(1, &colorTexture);
		glDeleteTextures(1, &heightTexture);
		glDeleteTextures(1, &normalTexture);
	}

	gltDeleteText(text);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

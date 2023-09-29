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
/* In this exercise you'll implement and compare the variants of Phong & Blinn-Phong
* we covered in the earlier lecture.
*
*  ** This time you'll only need to modify the shader CompareLighting.frag, not any of the
* C++ code here. **
*
* At the moment this application shows models without illumination, but sets up the
* required uniforms you'll need for the shader. These include a lightingMode integer
* that selects which lighting model to use.
* Implement the variants of Blinn-Phong from the slides, corresponding to the following
* values of lightingMode:
* 0: Classic Phong lighting (use the reflected vector)
* 1: Blinn-Phong (use the half-vector, but don't scale the specular term by n.l)
* 2: Modified Blinn-Phong (scale the specular term by n.l)
* 3: Modified Blinn-Phong, normalised (add the normalisation factor from the slides).
*
* You should model the light source as a point light and give it a 1/d^2 falloff.
* 
* If you've implemented it correctly, you should see that Blinn-Phong gives differently-shaped
* highlights to Phong (they should stretch out more at grazing angles). It'll be most noticeable
* on the flat plane.
* The difference between Blinn-Phong and Modified Blinn-Phong is more subtle, but the modified
* Blinn-Phong highlights should get dimmer where they are more distant from the light source.
* For the normalised variant, try increasing/decreasing the exponent with UP and DOWN. In your
* normalised version, lower exponent values should produce dimmer highlights (in the other variants
* the highlight intensity shouldn't change).
* 
* You can press the R key to turn on/off rotating the light source.
*
* Hints
* -----
* 
* Don't forget to normalise vectors where appropriate (e.g. half-vectors, light vector & view vector).
*/

const int winWidth = 1280, winHeight = 720;

const Uint64 desiredFrametime = 33;

float wrapAmount = 0.0f;
float theta = 0.0f;
bool lightRotating = false;
Eigen::Vector3f lightPos(5.f*sinf(theta), 5.f, 5.f*cosf(theta)), spherePos(5.f, 0.f, 0.f);
float lightIntensity = 60.f;
int lightingMode = 0;
Eigen::Vector4f albedo(0.1f, 0.6f, 0.6f, 1.f);
float specularExponent = 60.f, specularIntensity = 1.0f;

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

Eigen::Matrix4f makeTranslationMatrix(const Eigen::Vector3f& translate)
{
	Eigen::Matrix4f matrix = Eigen::Matrix4f::Identity();
	matrix.block<3, 1>(0, 3) = translate;
	return matrix;
}

void updateText(GLTtext* text)
{
	std::stringstream textStream;
	if (lightingMode == 0)
		textStream << "Classic Phong";
	else if (lightingMode == 1) 
		textStream << "Blinn-Phong";
	else if (lightingMode == 2)
		textStream << "Modified Blinn-Phong";
	else 
		textStream << "Modified Blinn-Phong, Normalised";
	textStream << " exp: " << specularExponent << " int: " << specularIntensity;
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
	updateText(text);
	glEnable(GL_MULTISAMPLE);

	{
		glhelper::ShaderProgram compareLightingShader({ "../shaders/CompareLighting.vert", "../shaders/CompareLighting.frag" });
		glhelper::ShaderProgram fixedColorShader({ "../shaders/FixedColor.vert", "../shaders/FixedColor.frag" });
		//glhelper::RotateViewer viewer(winWidth, winHeight);
		glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh spotMesh, sphereMesh, planeMesh;

        loadMesh(&spotMesh, "../models/spot/spot_triangulated.obj");
        loadMesh(&sphereMesh, "../models/sphere.obj");
        loadMesh(&planeMesh, "../models/plane.obj");
		spotMesh.shaderProgram(&compareLightingShader);
		planeMesh.shaderProgram(&compareLightingShader);
		sphereMesh.shaderProgram(&fixedColorShader);
		planeMesh.modelToWorld(Eigen::Matrix4f::Identity());
		spotMesh.modelToWorld(makeTranslationMatrix(Eigen::Vector3f(0.f, 0.7f, 0.f)));
		sphereMesh.modelToWorld(makeTranslationMatrix(lightPos));
		viewer.position(Eigen::Vector3f(0.f, -2.f, -5.f));

		std::vector < glhelper::Mesh* > meshes {&spotMesh, &sphereMesh, &planeMesh};

		glProgramUniform1i(compareLightingShader.get(), compareLightingShader.uniformLoc("lightingMode"), lightingMode);
		glProgramUniform4f(fixedColorShader.get(), fixedColorShader.uniformLoc("color"), 1.f, 1.f, 1.f, 1.f);
		glProgramUniform4f(compareLightingShader.get(),compareLightingShader.uniformLoc("albedo"), albedo.x(), albedo.y(), albedo.z(), albedo.w());
		glProgramUniform1f(compareLightingShader.get(), compareLightingShader.uniformLoc("specularExponent"), specularExponent);
		glProgramUniform1f(compareLightingShader.get(), compareLightingShader.uniformLoc("specularIntensity"), specularIntensity);
		glProgramUniform3f(compareLightingShader.get(), compareLightingShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());
		glProgramUniform1f(compareLightingShader.get(), compareLightingShader.uniformLoc("lightIntensity"), lightIntensity);

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
						lightingMode += 1;
						lightingMode %= 4;
						glProgramUniform1i(compareLightingShader.get(), compareLightingShader.uniformLoc("lightingMode"), lightingMode);
						updateText(text);
					}
					if (event.key.keysym.sym == SDLK_r) {
						lightRotating = !lightRotating;
					}
					if (event.key.keysym.sym == SDLK_UP) {
						if (event.key.keysym.mod & KMOD_SHIFT) {
							specularIntensity += 0.1f;
							glProgramUniform1f(compareLightingShader.get(), compareLightingShader.uniformLoc("specularIntensity"), specularIntensity);
							updateText(text);
						}
						else {
							specularExponent += 10.f;
							glProgramUniform1f(compareLightingShader.get(), compareLightingShader.uniformLoc("specularExponent"), specularExponent);
							updateText(text);
						}
					}
					if (event.key.keysym.sym == SDLK_DOWN) {
						if (event.key.keysym.mod & KMOD_SHIFT) {
							specularIntensity -= 0.1f;
							if (specularIntensity < 0.f) specularIntensity = 0.f;
							glProgramUniform1f(compareLightingShader.get(), compareLightingShader.uniformLoc("specularIntensity"), specularIntensity);
							updateText(text);
						}
						else {
							specularExponent -= 10.0f;
							if (specularExponent < 0.f) specularExponent = 0.f;
							glProgramUniform1f(compareLightingShader.get(), compareLightingShader.uniformLoc("specularExponent"), specularExponent);
							updateText(text);
						}
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
				glProgramUniform3f(compareLightingShader.get(), compareLightingShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());
			}

			for (auto mesh : meshes) {
				mesh->render();
			}

			glDisable(GL_CULL_FACE);

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

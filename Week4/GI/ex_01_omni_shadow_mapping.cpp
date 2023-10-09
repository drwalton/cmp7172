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
/* In this exercise you'll implement omnidirectional shadow mapping, using cubemaps.
* You'll change this code to create a cubemap depth texture. You'll then render the 
* shadow-casting objects in the scene to the six faces of this cubemap. 
* This will use the ShadowMap.vert and ShadowMap.frag shaders, which you should implement.
* You'll then implement the ShadowMapped.vert and ShadowMapped.frag shaders.
* These are used to render the walls of the room. At the moment they just do Lambertian
* shading, but you should also add shadow mapping here.
* 
* If you press the space key the light should rotate all around the bunny mesh - this will
* let you test your shadow map works from all angles.
* 
* As an extra optional task, if time allows, try implementing percentage closer filtering
* to get soft shadows. Note that the R key (with/without shift) lets you change the size
* of the sphere representing our light source. You should implement ShadowMappedPCF.frag
* to generate extra light source locations based on the current size of the sphere.
* It should then perform shadow tests for each light location and average the results.
* 
* Tasks
* -----
* 1. Create the cubemap texture, and a framebuffer to allow you to render to it.
* 2. Create an appropriate projection matrix for rendering each face of the cubemap.
* 3. When rendering the faces, pass in a suitable worldToClip matrix. This should translate
* based on the current light location, rotate based on the face being rendered, then apply
* the projection matrix.
* 4. Add code to perform the cubemap rendering in the main game loop. This should render all
* six faces. Only render the objects that should cast shadows (Renderable instances have a 
* castsShadows() method you can use to check if it should cast a shadow).
* 5. Implement the ShadowMapped.vert and ShadowMapped.frag shaders, which are used to render
* the walls. These should perform a shadow test using the cubemap and current light position.
* 6. Optionally also implement Percentage Closer Filtering (do this in ShadowMappedPCF.frag,
* and change the shader source that you load).
* 
* Hints
* -----
* 1. It can be hard to tell if your shadow maps are really rendering properly. RenderDoc
* can be great for debugging things like this where you don't draw to screen. You may
* want to check out the "Where's my Draw?" extension which has a kind of troubleshooter
* to tell why nothing is being drawn to screen (or to the cubemap in this case!)
* 2. Don't forget to set approprate viewports for rendering the cubemap (and change it 
* back afterwards!)
* 3. For PCF, perform multiple shadow tests & average the results, don't average depth values
* 4. Again for PCF, I generated samples in a grid in a plane perpendicular to the vector from 
* light to surface point. If you'd like to implement it this way, you can use cross products
* to get vectors in this perpendicular plane, and then space them out according to the light
* source size. There are other ways you could go about this though! Ask me if you want to
* discuss this.
* 5. If you run into shadow acne issues, add a small bias to make the test less "strict".
*/

const int winWidth = 1280, winHeight = 720;

const int cubemapSize = 512;

const Uint64 desiredFrametime = 33;

float wrapAmount = 0.0f;
float theta = 0.0f, phi = 0.0f;
const float lightRadius = 5.f;
bool lightRotating = false;
Eigen::Vector3f lightPos(lightRadius * sinf(theta) * cosf(phi), lightRadius * sinf(phi),  lightRadius * cosf(theta) * cosf(phi));
const float shadowMapNear = 0.5f, shadowMapFar = 100.f;
float shadowMapBias = 1.0f;
float lightWidth = 0.01f;
int sampleRadius = 1;

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


Eigen::Matrix4f angleAxisMat4(float angle, const Eigen::Vector3f& axis)
{
	Eigen::Matrix4f output = Eigen::Matrix4f::Identity();
	output.block<3, 3>(0, 0) = Eigen::AngleAxisf(angle, axis).matrix();
	return output;
}

void updateText(GLTtext* text)
{
	std::stringstream textStream;
	textStream << "Omni Shadow Mapping Bias " << shadowMapBias << " Light Radius " << lightWidth << " Sample kernel radius " << sampleRadius;
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
	gltSetText(text, (std::string("wrapAmount: ") + std::to_string(wrapAmount)).c_str());
	glEnable(GL_MULTISAMPLE);

	{
		glhelper::ShaderProgram bunnyShader({ "../shaders/LambertianTextured.vert", "../shaders/LambertianTextured.frag" });
		glhelper::ShaderProgram fixedColorShader({ "../shaders/FixedColor.vert", "../shaders/FixedColor.frag" });
		glhelper::ShaderProgram shadowMapShader({ "../shaders/ShadowMap.vert", "../shaders/ShadowMap.frag" });
		glhelper::ShaderProgram shadowMappedShader({ "../shaders/ShadowMapped.vert", "../shaders/ShadowMapped.frag" });
		//glhelper::ShaderProgram shadowMappedShader({ "../shaders/ShadowMapped.vert", "../shaders/ShadowMappedPCF.frag" });
		glhelper::RotateViewer viewer(winWidth, winHeight);
		//glhelper::FlyViewer viewer(winWidth, winHeight);
		glhelper::Mesh bunnyMesh, sphereMesh, roomMesh;

		std::vector<glhelper::Renderable*> scene{ &bunnyMesh, &roomMesh, &sphereMesh };

		Eigen::Matrix4f bunnyModelToWorld = Eigen::Matrix4f::Identity();
		bunnyModelToWorld(0, 0) = 0.2f;
		bunnyModelToWorld(1, 1) = 0.2f;
		bunnyModelToWorld(2, 2) = 0.2f;
		bunnyModelToWorld = makeTranslationMatrix(Eigen::Vector3f(0.f, -0.5f, 0.f)) * bunnyModelToWorld;

        loadMesh(&bunnyMesh, "../models/stanford_bunny/scene.gltf");
        loadMesh(&sphereMesh, "../models/sphere.obj");
        loadMesh(&roomMesh, "../models/bigcube.obj");
		bunnyMesh.modelToWorld(bunnyModelToWorld);
		bunnyMesh.shaderProgram(&bunnyShader);
		sphereMesh.shaderProgram(&fixedColorShader);
		sphereMesh.setCastsShadow(false); // The light source sphere shouldn't cast a shadow, 
		// as otherwise everything would always be in shadow!
		roomMesh.shaderProgram(&shadowMappedShader);
		sphereMesh.modelToWorld(makeTranslationMatrix(lightPos) * makeScaleMatrix(lightWidth));

		glProgramUniform1i(bunnyShader.get(), bunnyShader.uniformLoc("albedoTexture"), 0);
		glProgramUniform4f(fixedColorShader.get(), fixedColorShader.uniformLoc("color"), 1.f, 1.f, 1.f, 1.f);
		glProgramUniform1f(shadowMapShader.get(), shadowMapShader.uniformLoc("nearPlane"), shadowMapNear);
		glProgramUniform1f(shadowMapShader.get(), shadowMapShader.uniformLoc("farPlane"), shadowMapFar);

		glProgramUniform4f(shadowMappedShader.get(), shadowMappedShader.uniformLoc("color"), 0.1f, 0.8f, 0.8f, 1.f);
		glProgramUniform1f(shadowMappedShader.get(), shadowMappedShader.uniformLoc("nearPlane"), shadowMapNear);
		glProgramUniform1f(shadowMappedShader.get(), shadowMappedShader.uniformLoc("farPlane"), shadowMapFar);
		glProgramUniform1i(shadowMappedShader.get(), shadowMappedShader.uniformLoc("shadowMap"), 1);
		glProgramUniform1f(shadowMappedShader.get(), shadowMappedShader.uniformLoc("lightRadius"), lightWidth);
		glProgramUniform1i(shadowMappedShader.get(), shadowMappedShader.uniformLoc("sampleRadius"), sampleRadius);
		glProgramUniform1f(shadowMappedShader.get(), shadowMappedShader.uniformLoc("bias"), shadowMapBias);

		cv::Mat bunnyTextureImage = cv::imread("../models/stanford_bunny/textures/Bunny_baseColor.png");
		cv::cvtColor(bunnyTextureImage, bunnyTextureImage, cv::COLOR_BGR2RGB);
		glhelper::Texture bunnyTexture(
			GL_TEXTURE_2D, GL_RGB8, 
			bunnyTextureImage.cols, bunnyTextureImage.rows, 
			0, GL_RGB, GL_UNSIGNED_BYTE, bunnyTextureImage.data,
			GL_LINEAR_MIPMAP_LINEAR,
			GL_LINEAR);
		bunnyTexture.genMipmap();

		// Your Code Here
		// Create your cubemap texture here, and a framebuffer object to
		// allow you to render to it.
		// You'll need to call glTexImage2D 6 times this time, once per face.

		// Your Code Here
		// Set this matrix to be an appropriate projection matrix for rendering your cubemaps.
		// Feel free to use the functions in glhelper's Matrices.hpp.
		Eigen::Matrix4f cubemapPerspective;

		// Here are the rotations for each face of the cubemap (please do use them!)
		const std::array<Eigen::Matrix4f, 6> cubemapRotations {
			angleAxisMat4(float(M_PI_2), Eigen::Vector3f(0,1,0)),//POSITIVE_X - rotate right 90 degrees
			angleAxisMat4(float(-M_PI_2), Eigen::Vector3f(0,1,0)),//NEGATIVE_X - rotate left 90 degrees
			angleAxisMat4(float(-M_PI_2), Eigen::Vector3f(1,0,0))* angleAxisMat4(float(M_PI), Eigen::Vector3f(0,1,0)),//POSITIVE_Y - rotate up 90 degrees
			angleAxisMat4(float(M_PI_2), Eigen::Vector3f(1,0,0))* angleAxisMat4(float(M_PI), Eigen::Vector3f(0,1,0)),//NEGATIVE_Y - rotate down 90 degrees
			angleAxisMat4(float(M_PI), Eigen::Vector3f(0,1,0)),     //POSITIVE_Z - rotate right 180 degrees
			Eigen::Matrix4f::Identity()                           //NEGATIVE_Z
		};

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
						lightRotating = !lightRotating;
					}
					if (event.key.keysym.sym == SDLK_r) {
						if (event.key.keysym.mod & KMOD_SHIFT) {
							lightWidth -= 0.01f;
							lightWidth = std::max(lightWidth, 0.f);
						}
						else {
							lightWidth += 0.01f;
						}
						glProgramUniform1f(shadowMappedShader.get(), shadowMappedShader.uniformLoc("lightRadius"), lightWidth);
						sphereMesh.modelToWorld(makeTranslationMatrix(lightPos) * makeScaleMatrix(lightWidth));
						updateText(text);
					}
					if (event.key.keysym.sym == SDLK_s) {
						if (event.key.keysym.mod & KMOD_SHIFT) {
							sampleRadius -= 1;
							sampleRadius = std::max(sampleRadius, 1);
						}
						else {
							sampleRadius += 1;
						}
						glProgramUniform1i(shadowMappedShader.get(), shadowMappedShader.uniformLoc("sampleRadius"), sampleRadius);
						updateText(text);
					}
					if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN) {
						if (event.key.keysym.sym == SDLK_UP) {
							shadowMapBias += 0.01f;
						}
						else {
							shadowMapBias -= 0.01f;
						}
						glProgramUniform1f(shadowMappedShader.get(), shadowMappedShader.uniformLoc("bias"), shadowMapBias);
						updateText(text);
					}
				}

				// --- Your code here ---
				// You could add your controls to adjust the wrapLighting effect here
			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (lightRotating) {
				theta += 0.01f;
				phi += 0.03f;
				if (theta > 2 * 3.14159f) theta = 0.f;
				if (phi > 2 * 3.14159f) phi = 0.f;
				lightPos << lightRadius*sinf(theta)*cosf(phi), lightRadius * sinf(phi),  lightRadius* cosf(theta)*cosf(phi);
				sphereMesh.modelToWorld(makeTranslationMatrix(lightPos) * makeScaleMatrix(lightWidth));
			}

			glProgramUniform3f(shadowMapShader.get(), shadowMapShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());
			glProgramUniform3f(shadowMappedShader.get(), shadowMappedShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());
			glProgramUniform3f(bunnyShader.get(), bunnyShader.uniformLoc("lightPosWorld"), lightPos.x(), lightPos.y(), lightPos.z());

			// Your Code Here
			// Bind your shadow framebuffer, set up your viewport, and render the 6 faces of the cubemap.
			// For each face set the value of shadowWorldToClip in the shader appropriately.
			// This should be based on the current light position, face rotation and cubemap perspective
			// matrix.
			// Before rendering the main scene don't forget to unbind your framebuffer and set
			// the viewport back to normal.

			bunnyTexture.bindToImageUnit(0);
			// Your Code Here
			// Also bind your created shadow cubemap texture here, to image unit 1.
			for (glhelper::Renderable* mesh : scene) {
				mesh->render();
			}

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
		// Your Code Here
		// Don't forget to delete your cubemap texture.
	}


	gltDeleteText(text);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

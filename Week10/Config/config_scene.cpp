#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <iostream>
#include <fstream>
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

#include "json/json.hpp"

const int winWidth = 1280, winHeight = 720;

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

	// Prepare window1.f, 1.f, 1.f, 1.f
	SDL_Window* window = SDL_CreateWindow("Geometry Shader-Based Fire Exercise", 50, 50, winWidth, winHeight, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);

	// Use adaptive vsync if possible, else regular vsync.
	if (SDL_GL_SetSwapInterval(-1) != 0)
		SDL_GL_SetSwapInterval(1);

	GLenum result = glewInit();
	if (result != GLEW_OK) {
		throw std::runtime_error("GLEW couldn't initialize.");
	}

	gltInit();
	GLTtext* text = gltCreateText();
	gltSetText(text, (std::string("Animation demo")).c_str());
	glEnable(GL_MULTISAMPLE);

	{
		nlohmann::json data;
		try {
			std::ifstream jsonFile("../config/config.json");
			data = nlohmann::json::parse(jsonFile);
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
			exit(1);
		}

		// Load all the meshes
		std::map<std::string, glhelper::Mesh> meshes;
		for (auto& mesh : data["meshes"]) {
			std::cout << mesh["name"] << " " << mesh["filename"] << std::endl;
			std::string meshName = mesh["name"];
			meshes.emplace(std::piecewise_construct, std::forward_as_tuple(meshName), std::forward_as_tuple());
			loadMesh(&(meshes[mesh["name"]]), mesh["filename"]);
		}

		// Load all the shaders
		std::map<std::string, glhelper::ShaderProgram> shaders;
		for (auto& shader : data["shaders"]) {
			std::vector<std::string> sourceFilenames;
			for (auto& filename : shader["filenames"]) {
				sourceFilenames.push_back(filename);
			}

			std::string shaderName = shader["name"];
			shaders.emplace(shaderName, sourceFilenames);

		}

		// Load all the textures
		std::map<std::string, glhelper::Texture> textures;
		for (auto& texture : data["textures"]) {
			std::string textureName = texture["name"];
			cv::Mat image = cv::imread(texture["filename"]);
			textures.emplace(textureName, glhelper::Texture{ GL_TEXTURE_2D, GL_RGB8, (size_t)image.cols, (size_t)image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data });
		}

		nlohmann::json models = data["models"];


		glhelper::RotateViewer viewer(winWidth, winHeight);
		viewer.distance(20.f);

		glhelper::Mesh sphereMesh;
		loadMesh(&sphereMesh, "../models/sphere.obj");
		sphereMesh.shaderProgram(&(shaders.at("texturedMesh")));

		glProgramUniform1i(shaders.at("texturedMesh").get(), shaders.at("texturedMesh").uniformLoc("tex"), 0);

		bool shouldQuit = false;
		SDL_Event event;

		glEnable(GL_DEPTH_TEST);

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
			}
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			for (auto& model : models) {
				glhelper::Mesh& mesh = meshes.at(model["mesh"]);
				glhelper::ShaderProgram& shader = shaders.at(model["shader"]);
				glhelper::Texture& texture = textures.at(model["texture"]);
				texture.bindToImageUnit(0);
				glProgramUniform1i(shader.get(), shader.uniformLoc("tex"), 0);
				mesh.shaderProgram(&shader);

				Eigen::Vector3f position(model["position"][0], model["position"][1], model["position"][2]);
				mesh.modelToWorld(makeTranslationMatrix(position));
				mesh.render();
			}

			std::string textStr = std::string("Animation Time: ") + std::to_string(animTimeSeconds) + std::string(" seconds.");
			gltSetText(text, textStr.c_str());
			gltBeginDraw();
			gltColor(1.f, 1.f, 1.f, 1.f);
			gltDrawText2D(text, 10.f, 10.f, 1.f);
			gltEndDraw();

			SDL_GL_SwapWindow(window);
		}

	}

	gltDeleteText(text);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

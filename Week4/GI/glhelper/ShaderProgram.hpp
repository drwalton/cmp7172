#pragma once

#include <GL/glew.h>
#include <string>
#include <vector>
#include <map>
#include <set>

namespace glhelper {

void checkForGLError(const std::string &exception = "OpenGL error encountered!");

std::string glErrToString(GLenum err);

//!\brief Class representing a compiled and linked OpenGL shader program.
class ShaderProgram final
{
public:
	enum class TFMode {INTERLEAVED_ATTRIBS, SEPARATE_ATTRIBS};

	ShaderProgram(const std::vector<std::string> &sources,
		const std::vector<std::string> *transformFeedbackVaryings = nullptr,
		TFMode tfMode = TFMode::INTERLEAVED_ATTRIBS);
	ShaderProgram(ShaderProgram &&);
	ShaderProgram &operator=(ShaderProgram &&);
	~ShaderProgram() throw();
	GLint uniformLoc(const std::string &name);

	void use();
	void unuse();

	void bindTransformFeedbackBuffer(size_t idx, GLuint buffer);

	void setupUniformBlock(const std::string &name, GLuint index);
	void setupCameraBlock();

	//!\brief Validate shader program. Throws ShaderException on failure, which 
	//!       contains details of the nature of the failure.
	void validate();

	GLuint get() const { return program_; }
private:
	ShaderProgram(const ShaderProgram &);
	ShaderProgram &operator=(const ShaderProgram &);

	std::map<std::string, GLint> uniforms_;
	std::set<std::string> notfound_;
	GLuint program_;

	static std::string preprocessSource(const std::string &source, const std::string &sourcePath);
	static GLuint compileShader(const std::string &filename, GLenum type);
	static GLuint makeShaderProgram(
		const std::vector<GLuint> &shaders,
		const std::vector<std::string> *tfFeedbackVaryings,
		TFMode tfMode);
	static GLuint makeShaderProgram(
		const std::vector<std::string> &files,
		const std::vector<GLenum> &types,
		const std::vector<std::string> *tfFeedbackVaryings,
		TFMode tfMode);
	static bool setupUniformBlock(
		GLuint shaderProgram, const std::string &name, GLuint index);
	static void setupCameraBlock(GLuint shaderProgram);
	static std::string framebufferErrorToString(int errcode);
	static GLint getUniformLocation(GLuint program, const std::string &name);
	static GLenum shaderTypeFromFilename(const std::string &filename);

	std::string filenames_;
};

}


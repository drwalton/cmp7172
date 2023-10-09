#include "ShaderProgram.hpp"
#include <Eigen/Dense>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <iostream>
#include <fstream>
#include <filesystem>

namespace glhelper {


std::string getFileContents(const std::string& filename)
{
	std::ifstream file(filename);
	if (file.fail()) throw std::runtime_error(
		"Could not open file \"" + filename + "\".");
	std::stringstream buff;
	buff << file.rdbuf();
	return buff.str();
}

void checkForGLError(const std::string &exception)
{
	GLenum err = glGetError();
	std::string errname = glErrToString(err);
	if (err != GL_NO_ERROR) {
		//throw std::runtime_error(exception + " " + errname);
	}
}

std::string glErrToString(GLenum err)
{
	switch (err) {
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
	}
	return "UNKNOWN_GL_ERROR_ENUM";
}

ShaderProgram::ShaderProgram(
	const std::vector<std::string> &sources,
	const std::vector<std::string> *transformFeedbackVaryings,
	TFMode tfMode)
	:program_(0)
{
	std::vector<GLenum> types;
	std::stringstream filenames;
	for(const std::string &s : sources) {
		types.push_back(shaderTypeFromFilename(s));
	}
	program_ = makeShaderProgram(sources, types, transformFeedbackVaryings, tfMode);

	filenames << "\"" << sources[0] << "\"";
	for (size_t i = 1; i < sources.size(); ++i) {
		filenames << ", \"" << sources[i] << "\"";
	}
	filenames_ = filenames.str();
}

ShaderProgram::ShaderProgram(ShaderProgram &&other)
{
	this->program_ = other.program_;
	other.program_ = 0;
}

ShaderProgram & ShaderProgram::operator=(ShaderProgram &&other)
{
	this->program_ = other.program_;
	other.program_ = 0;
	return *this;
}

ShaderProgram::~ShaderProgram() throw()
{
	if (program_ != 0) {
		glDeleteProgram(program_);
	}
}

void ShaderProgram::use()
{
	glUseProgram(program_);
}

void ShaderProgram::unuse()
{
	glUseProgram(0);
}

void ShaderProgram::bindTransformFeedbackBuffer(size_t idx, GLuint buffer)
{
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, GLuint(idx), buffer);
}

void ShaderProgram::setupUniformBlock(const std::string &name, GLuint index)
{
	setupUniformBlock(program_, name, index);
}

void ShaderProgram::setupCameraBlock()
{
	setupCameraBlock(program_);
}

void ShaderProgram::validate()
{
	glValidateProgram(program_);
	int val;
	glGetProgramiv(program_, GL_VALIDATE_STATUS, &val);
	if(val != GL_TRUE) {
		GLint logLength = 0;
		glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &logLength);
		GLchar* errorLog = new GLchar[logLength];
		glGetProgramInfoLog(program_, logLength, 0, errorLog);
		std::string errorString(errorLog);
		delete[] errorLog;
		throw std::runtime_error("Program validation failed!: \"" +
			errorString + "\"");
	}
}

GLint ShaderProgram::uniformLoc(const std::string &name)
{
	GLint loc;
	if(uniforms_.count(name)) {
		loc = uniforms_[name];
	} else {
		loc = getUniformLocation(program_, name);
		if(loc == -1 || glGetError() != GL_NO_ERROR) {
			
    		if(!notfound_.count(name)) {
    			std::cout << "Warning: couldn't find uniform \"" << name <<
    				"\" in the program compiled from the source files " << 
					filenames_ << "." << std::endl;
				notfound_.insert(name);
			}
		} else {
			//Found without error.
			uniforms_.insert(std::make_pair(name, loc));
		}
	}
	return loc;
}

GLuint ShaderProgram::compileShader(const std::string &filename, GLenum type)
{
	GLuint shader = glCreateShader(type);
	std::string source = getFileContents(filename);
	std::string preprocessedSource = ShaderProgram::preprocessSource(source, filename);
	const char* cSource = preprocessedSource.c_str();
	glShaderSource(shader, 1, &cSource, 0);
	glCompileShader(shader);
	
	GLint status;
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) //Compilation failure.
	{
		GLint logLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		GLchar* errorLog = new GLchar[logLength];
		glGetShaderInfoLog(shader, logLength, 0, errorLog);
		std::string errorString(errorLog);
		delete[] errorLog;

		std::cout << "Failed to compile following shader source:\n";

		std::istringstream f(preprocessedSource);
		std::string line;
		uint32_t lineNo = 0;
		while (std::getline(f, line)) {
			std::cout << lineNo++ << "\t" << line << "\n";
		}

		std::cout << "With error:\n" 
			<< errorString << std::endl;
		throw std::runtime_error("The shader source obtained from\n"
						  "    file: " + filename + "\n"
						  "could not be compiled.\n" + errorString);
	}
	
	return shader;
}

GLuint ShaderProgram::makeShaderProgram(
	const std::vector<GLuint> &shaders,
	const std::vector<std::string> *tfFeedbackVaryings,
	TFMode tfMode)
{
	GLuint program = glCreateProgram();

	glBindFragDataLocation(program, 0, "color");
	
	for (GLuint shader : shaders) {
		glAttachShader(program, shader);
	}

	if (tfFeedbackVaryings != nullptr) {
		GLchar **varyingsArr = new GLchar*[tfFeedbackVaryings->size()];
		for (size_t i = 0; i < tfFeedbackVaryings->size(); ++i) {
			varyingsArr[i] = const_cast<GLchar*>((*tfFeedbackVaryings)[i].c_str());
		}
		GLenum mode = (tfMode == TFMode::INTERLEAVED_ATTRIBS) ? GL_INTERLEAVED_ATTRIBS : GL_SEPARATE_ATTRIBS;
		glTransformFeedbackVaryings(program, tfFeedbackVaryings->size(), varyingsArr, mode);
		delete[] varyingsArr;
	}
	
	glLinkProgram(program);
	
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) //Linking failure
	{
		GLint logLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		GLchar* errorLog = new GLchar[logLength];
		glGetProgramInfoLog(program, logLength, 0, errorLog);
		std::string errorString(errorLog);
		delete[] errorLog;
		std::cout << "Error encountered linking shaders: \n" <<
			errorString << std::endl;
		throw(std::runtime_error(("Error encountered when linking shaders: " +
								  errorString).c_str()));
	}
	checkForGLError("Error encountered linking program.");
	
	return program;
}

GLuint ShaderProgram::makeShaderProgram(
	const std::vector<std::string> &files,
	const std::vector<GLenum> &types,
	const std::vector<std::string> *tfFeedbackVaryings,
	TFMode tfMode)
{
	if (files.size() != types.size()) {
		throw std::runtime_error(
			"makeShaderProgram needs same no. of shader files, shader types.");
	}
	
	std::vector<GLuint> shaders(files.size());
	for (size_t i = 0; i < files.size(); ++i) {
		shaders[i] = compileShader(files[i], types[i]);
		checkForGLError("Error encountered compiling shaders.");
	}
	
	try {
		GLuint program = makeShaderProgram(shaders, tfFeedbackVaryings, tfMode);
		for (GLuint shader : shaders) {
			glDetachShader(program, shader);
			glDeleteShader(shader);
		}
		return program;
	}
	catch (std::runtime_error &e) {
		std::stringstream errString;
		errString << e.what() << "\n" <<
		"Shaders: \n";
		for (const std::string &file : files) {
			errString << "\t" << file << "\n";
		}
		std::cout << "Shader linking failed with error:\n" 
			<< errString.str() << std::endl;
		throw std::runtime_error(errString.str().c_str());
	}
}

bool ShaderProgram::setupUniformBlock(GLuint shaderProgram, const std::string &name, GLuint index)
{
	GLuint ublockIndex = glGetUniformBlockIndex(shaderProgram, name.c_str());
	if (ublockIndex == GL_INVALID_INDEX) {
		return false;
	}
	glUniformBlockBinding(shaderProgram, ublockIndex, index);
	return true;
}

void ShaderProgram::setupCameraBlock(GLuint shaderProgram)
{
	setupUniformBlock(shaderProgram, "cameraBlock", 0);
}

std::string ShaderProgram::framebufferErrorToString(int errcode)
{
	switch (errcode)
	{
		case GL_FRAMEBUFFER_UNDEFINED:
			return "GL_FRAMEBUFFER_UNDEFINED: "
			"returned if target is the default framebuffer,"
			" but the default framebuffer does not exist.";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: "
			"returned if any of the framebuffer attachment"
			" points are framebuffer incomplete.";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: "
			"returned if the framebuffer does not have"
			" at least one image attached to it.";
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: "
			"returned if the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE "
			"is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi.";
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: "
			"returned if GL_READ_BUFFER is not GL_NONE"
			" and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is"
			" GL_NONE for the color attachment point named"
			" by GL_READ_BUFFER.";
		case GL_FRAMEBUFFER_UNSUPPORTED:
			return "GL_FRAMEBUFFER_UNSUPPORTED: "
			"returned if the combination of internal formats of"
			" the attached images violates"
			"an implementation-dependent set of restrictions.";
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: "
			"returned if the value of GL_RENDERBUFFER_SAMPLES is not the same"
			"for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES"
			" is the not same for all attached textures; or, if the attached"
			"images are a mix of renderbuffers and textures, the value of"
			" GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES."
			"also returned if the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is"
			"not the same for all attached textures; or, if the attached"
			" images are a mix of renderbuffers and textures, the value "
			"of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS"
			"is not GL_TRUE for all attached textures.";
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: "
			"returned if any framebuffer attachment is layered, and any"
			" populated attachment is not layered,"
			"or if all populated color attachments are not"
			" from textures of the same target.";
		case 0:
			return "Error encountered checking framebuffer completeness";
			
	}
	
	throw std::runtime_error(("No such framebuffer error code \""
							  + std::to_string(errcode) + "\"!").c_str());
}

GLint ShaderProgram::getUniformLocation(GLuint program, const std::string &name)
{
	GLint loc = glGetUniformLocation(program, name.c_str());
	checkForGLError("Error encountered in getting uniform\"" +
					name + "\" in program id \"" + std::to_string(program) + "\"");
	return loc;
}

GLenum ShaderProgram::shaderTypeFromFilename(const std::string &filename)
{
	size_t pos = filename.find_last_of(".");
	if(pos == std::string::npos) {
		throw std::runtime_error("Supplied shader filename did not"
								 " have recognised extension.");
	}
	std::string extension = filename.substr(pos+1, filename.length());
	if(extension == "vert") {
		return GL_VERTEX_SHADER;
	} else if(extension == "geom") {
		return GL_GEOMETRY_SHADER;
	} else if(extension == "frag") {
		return GL_FRAGMENT_SHADER;
	} else if(extension == "tesc") {
		return GL_TESS_CONTROL_SHADER;
	} else if(extension == "tese") {
		return GL_TESS_EVALUATION_SHADER;
	} else if(extension == "comp") {
		return GL_COMPUTE_SHADER;
	} else {
		throw std::runtime_error("Supplied shader filename did not"
								 " have recognised extension.");
	}
}

std::string ShaderProgram::preprocessSource(const std::string &source, const std::string &sourcePath)
{
	std::stringstream processedSource;
	std::stringstream inputSource(source);

	std::string currLine;
	while (std::getline(inputSource, currLine, '\n')) {
		const std::string includeString = "#pragma include ";
		if (currLine.compare(0, includeString.length(), includeString) == 0) {
			std::string filename = currLine.substr(includeString.length(), std::string::npos);
			std::filesystem::path p(sourcePath);
			std::filesystem::path dir = p.parent_path();
			std::string toInclude = getFileContents(dir.string() + "/" + filename);
			std::string toIncludeProcessed = preprocessSource(toInclude, sourcePath);
			processedSource << "\n" << toIncludeProcessed << "\n";
		}
		else {
			processedSource << currLine << "\n";
		}
	}

	return processedSource.str();
}

}

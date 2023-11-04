#include "Texture.hpp"
#include "Exception.hpp"
#include <GL/glew.h>
#include <string>
#include <opencv2/opencv.hpp>

bool validFormat(GLenum format)
{
	switch (format) {
	case GL_RED:
	case GL_RG: 
	case GL_RGB: 
	case GL_BGR: 
	case GL_RGBA: 
	case GL_BGRA: 
	case GL_RED_INTEGER: 
	case GL_RG_INTEGER: 
	case GL_RGB_INTEGER: 
	case GL_BGR_INTEGER: 
	case GL_RGBA_INTEGER: 
	case GL_BGRA_INTEGER: 
	case GL_STENCIL_INDEX: 
	case GL_DEPTH_COMPONENT: 
	case GL_DEPTH_STENCIL:
		return true;
	default:
		return false;
	}
}

bool validType(GLenum type)
{
	switch (type) {
	case GL_UNSIGNED_BYTE: 
	case GL_BYTE: 
	case GL_UNSIGNED_SHORT: 
	case GL_SHORT: 
	case GL_UNSIGNED_INT: 
	case GL_INT: 
	case GL_FLOAT: 
	case GL_UNSIGNED_BYTE_3_3_2: 
	case GL_UNSIGNED_BYTE_2_3_3_REV: 
	case GL_UNSIGNED_SHORT_5_6_5: 
	case GL_UNSIGNED_SHORT_5_6_5_REV: 
	case GL_UNSIGNED_SHORT_4_4_4_4: 
	case GL_UNSIGNED_SHORT_4_4_4_4_REV: 
	case GL_UNSIGNED_SHORT_5_5_5_1: 
	case GL_UNSIGNED_SHORT_1_5_5_5_REV: 
	case GL_UNSIGNED_INT_8_8_8_8: 
	case GL_UNSIGNED_INT_8_8_8_8_REV: 
	case GL_UNSIGNED_INT_10_10_10_2: 
	case GL_UNSIGNED_INT_2_10_10_10_REV:
			return true;
	default:
		return false;
	}
}

bool validInternalFormat(GLenum format)
{
	switch (format) {
		//Base internal formats
	case GL_DEPTH_COMPONENT:
	case GL_DEPTH_STENCIL:
	case GL_RED:
	case GL_RG:
	case GL_RGB:
	case GL_RGBA:
		//Sized internal formats
	case GL_R8:
	case GL_R8_SNORM:
	case GL_R16:
	case GL_R16_SNORM:
	case GL_RG8:
	case GL_RG8_SNORM:
	case GL_RG16:
	case GL_RG16_SNORM:
	case GL_R3_G3_B2:
	case GL_RGB4:
	case GL_RGB5:
	case GL_RGB8:
	case GL_RGB8_SNORM:
	case GL_RGB10:
	case GL_RGB12:
	case GL_RGB16_SNORM:
	case GL_RGBA2:
	case GL_RGBA4:
	case GL_RGB5_A1:
	case GL_RGBA8:
	case GL_RGBA8_SNORM:
	case GL_RGB10_A2:
	case GL_RGB10_A2UI:
	case GL_RGBA12:
	case GL_RGBA16:
	case GL_SRGB8:
	case GL_SRGB8_ALPHA8:
	case GL_R16F:
	case GL_RG16F:
	case GL_RGB16F:
	case GL_RGBA16F:
	case GL_R32F:
	case GL_RG32F:
	case GL_RGB32F:
	case GL_RGBA32F:
	case GL_R11F_G11F_B10F:
	case GL_RGB9_E5:
	case GL_R8I:
	case GL_R8UI:
	case GL_R16I:
	case GL_R16UI:
	case GL_R32I:
	case GL_R32UI:
	case GL_RG8I:
	case GL_RG8UI:
	case GL_RG16I:
	case GL_RG16UI:
	case GL_RG32I:
	case GL_RG32UI:
	case GL_RGB8I:
	case GL_RGB8UI:
	case GL_RGB16I:
	case GL_RGB16UI:
	case GL_RGB32I:
	case GL_RGB32UI:
	case GL_RGBA8I:
	case GL_RGBA8UI:
	case GL_RGBA16I:
	case GL_RGBA16UI:
	case GL_RGBA32I:
	case GL_RGBA32UI:

		//Compressed internal formats
	case GL_COMPRESSED_RED:
	case GL_COMPRESSED_RG:
	case GL_COMPRESSED_RGB:
	case GL_COMPRESSED_RGBA:
	case GL_COMPRESSED_SRGB:
	case GL_COMPRESSED_SRGB_ALPHA:
	case GL_COMPRESSED_RED_RGTC1:
	case GL_COMPRESSED_SIGNED_RED_RGTC1:
	case GL_COMPRESSED_RG_RGTC2:
	case GL_COMPRESSED_SIGNED_RG_RGTC2:
	case GL_COMPRESSED_RGBA_BPTC_UNORM:
	case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
	case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
	case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
		return true;
	default:
		return false;
	}
}

namespace glhelper {

Texture::Texture(GLenum target, GLenum internalFormat, 
	size_t width, size_t height, 
	GLint border, GLenum format, GLenum type,
	const GLvoid *data, GLenum minFilter, GLenum magFilter)
	:width_(width), height_(height),
	target_(target),
	internalFormat_(internalFormat),
	format_(format),
	type_(type),
	border_(border)
{
	if (!validFormat(format)) {
		throw std::runtime_error("Invalid format supplied to Texture constructor");
	}
	if (!validInternalFormat(internalFormat)) {
		throw std::runtime_error("Invalid internal format supplied to Texture constructor");
	}
	if (!validType(type)) {
		throw std::runtime_error("Invalid type supplied to Texture constructor");
	}
	init(data, minFilter, magFilter);
}

void Texture::init(const void *data, GLenum minFilter, GLenum magFilter)
{
	throwOnGlError();
	glActiveTexture(GL_TEXTURE0);
	throwOnGlError();
	glGenTextures(1, &tex_);
	glBindTexture(target_, tex_);
	glTexImage2D(target_, 0, internalFormat_, GLsizei(width_), GLsizei(height_),
		border_, format_, type_, data);
	throwOnGlError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
	throwOnGlError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	throwOnGlError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	throwOnGlError();
}

Texture::~Texture() throw()
{
	if (tex_ != 0) {
		glDeleteTextures(1, &tex_);
	}
}

Texture::Texture(Texture && other)
:width_(other.width_),
height_(other.height_),
target_(other.target_),
internalFormat_(other.internalFormat_),
format_(other.format_),
type_(other.type_),
border_(other.border_),
tex_(other.tex_)
{
	//Make sure tex_ isn't deleted.
	other.tex_ = 0;
}

Texture &Texture::operator=(Texture && other)
{
	width_ = other.width_;
	height_ = other.height_;
	target_ = other.target_;
	internalFormat_ = other.internalFormat_;
	format_ = other.format_;
	type_ = other.type_;
	border_ = other.border_;
	tex_ = other.tex_;
	//Make sure tex_ isn't deleted.
	other.tex_ = 0;
	return *this;
}

void Texture::update(const void *data)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(target_, tex_);
	glTexSubImage2D(target_, 0, 0, 0, width_, height_, format_, type_, data);
	glBindTexture(target_, 0);
}

void Texture::update(const void *data, size_t mipmapLevel) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(target_, tex_);
	glTexSubImage2D(target_, mipmapLevel, 0, 0, width_ >> mipmapLevel, height_ >> mipmapLevel, format_, type_, data);
	glBindTexture(target_, 0);
}

void Texture::bindToImageUnit(GLuint unit)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(target_, tex_);
}

void Texture::unbind()
{
	glBindTexture(target_, 0);
}

GLuint Texture::tex()
{
	return tex_;
}

void Texture::getData(void *data, size_t buffSize)
{
	#ifdef __APPLE__
	glBindTexture(GL_TEXTURE_2D, tex_);
	glGetTexImage(GL_TEXTURE_2D, 0, format_, type_, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	#else
	glGetTextureImage(tex_, 0, format_, type_, buffSize, data);
	#endif
}

void Texture::getData(void * data, size_t mipmapLevel, size_t buffSize)
{
	#ifdef __APPLE__
	glBindTexture(GL_TEXTURE_2D, tex_);
	glGetTexImage(GL_TEXTURE_2D, mipmapLevel, format_, type_, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	#else
	glGetTextureImage(tex_, mipmapLevel, format_, type_, buffSize, data);
	#endif
}

void Texture::saveToFile(const std::string & filepath)
{
	int type;
	size_t dataSize;
	if (type_ == GL_FLOAT) {
		if (format_ == GL_RGB) {
			type = CV_32FC3;
			dataSize = width()*height() * sizeof(float) * 3;
		} else if (format_ == GL_RED) {
			type = CV_32FC1;
			dataSize = width()*height() * sizeof(float) * 1;
		} else if (format_ == GL_RGBA) {
			type = CV_32FC4;
			dataSize = width()*height() * sizeof(float) * 4;
		}
	} else if (type_ == GL_UNSIGNED_BYTE) {
		if (format_ == GL_RGB) {
			type = CV_8UC3;
			dataSize = width()*height() * sizeof(GLubyte) * 3;
		} else if (format_ == GL_RED) {
			type = CV_8UC1;
			dataSize = width()*height() * sizeof(GLubyte) * 1;
		} else if (format_ == GL_RGBA) {
			type = CV_8UC4;
			dataSize = width()*height() * sizeof(GLubyte) * 4;
		}
	}
	cv::Mat mat(height(), width(), type);
	getData(mat.data, dataSize);
	cv::imwrite(filepath, mat);
}

size_t Texture::width() const
{
	return width_;
}
size_t Texture::height() const
{
	return height_;
}
size_t Texture::maxLevels() const
{
	int maxLevels;
#ifdef __APPLE__
	glBindTexture(GL_TEXTURE_2D, tex_);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, &maxLevels);
	glBindTexture(GL_TEXTURE_2D, 0);
#else
	
	glGetTextureParameteriv(tex_, GL_TEXTURE_MAX_LEVEL, &maxLevels);
#endif
	return (size_t)maxLevels;
}
size_t Texture::nTexels() const
{
	return width_*height_;
}
float Texture::aspect() const
{
	return static_cast<float>(width()) / static_cast<float>(height());
}
size_t Texture::bytesPerPixel() const
{
	return numChannels() * bytesPerChannel();
}

size_t Texture::bytesPerChannel() const
{
	switch (type_) {
	case GL_UNSIGNED_BYTE:
		return 1;
	case GL_FLOAT:
		return 4;
	default:
		throw std::runtime_error("NOT IMPLEMENTED");
	}
}

void Texture::genMipmap()
{
	glBindTexture(target_, tex_);
	glGenerateMipmap(target_);
	glBindTexture(target_, 0);
}

size_t Texture::numChannels() const
{
	switch (format_) {
	case GL_RGB:
		return 3;
	case GL_RGBA:
		return 4;
	default:
		throw std::runtime_error("NOT IMPLEMENTED");
	}
}

}

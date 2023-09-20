#pragma once

#include <GL/glew.h>
#include <vector>
#include <Eigen/Dense>

namespace glhelper {

class BufferObject {
public:
	BufferObject(size_t sizeBytes, GLenum usage = GL_DYNAMIC_DRAW, const void *data = nullptr);
	BufferObject(BufferObject &&tmp);
	virtual ~BufferObject() throw();

	void update(const void *data);
	void update(const void *data, size_t sizeBytes);
	template<typename T>
	void update(const std::vector<T> &v)
	{
    	glBindBuffer(GL_ARRAY_BUFFER, buf_);
    	glBufferSubData(GL_ARRAY_BUFFER, 0, v.size() * sizeof(T), v.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	template<typename T>
	void update(const std::vector<T> &v, size_t offset)
	{
    	glBindBuffer(GL_ARRAY_BUFFER, buf_);
    	glBufferSubData(GL_ARRAY_BUFFER, offset*sizeof(T), v.size() * sizeof(T), v.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void getData(void *data);
	void getData(void *data, size_t bytesToGet);
	void getData(void *data, size_t offset, size_t bytesToGet);

	GLuint get() { return buf_; }

	size_t sizeBytes() const;


private:
	BufferObject(const BufferObject&);
	BufferObject &operator=(const BufferObject&);
	GLuint buf_;
	size_t sizeBytes_;
};

//!\brief Class managing an OpenGL buffer object.
//!\note Should only be created when a GL context is active!
class VertexBuffer final : public BufferObject
{
public:
	explicit VertexBuffer(const std::vector<Eigen::Vector4f> &v, GLenum usage = GL_STATIC_DRAW);
	explicit VertexBuffer(const std::vector<Eigen::Vector3f> &v, GLenum usage = GL_STATIC_DRAW);
	explicit VertexBuffer(const std::vector<Eigen::Vector2f> &v, GLenum usage = GL_STATIC_DRAW);
	explicit VertexBuffer(const std::vector<float> &v, GLenum usage = GL_STATIC_DRAW);
	explicit VertexBuffer(const size_t size, GLenum usage = GL_STATIC_DRAW);
	VertexBuffer(VertexBuffer &&tmp);

	~VertexBuffer() throw();

	void bind();
	void unbind();
private:
	VertexBuffer(const VertexBuffer&);
	VertexBuffer &operator=(const VertexBuffer&);
};

//!\brief Class managing an OpenGL element buffer object.
//!\note Should only be created when a GL context is active!
class ElementBuffer final : public BufferObject
{
public:
	explicit ElementBuffer(const std::vector<GLuint> &v, GLenum usage);
	explicit ElementBuffer(const std::vector<GLushort> &v, GLenum usage);
	explicit ElementBuffer(size_t size, GLenum usage = GL_STATIC_DRAW);
	ElementBuffer(ElementBuffer &&tmp);

	~ElementBuffer() throw();

	void bind();
	void unbind();
private:
	ElementBuffer(const ElementBuffer&);
	ElementBuffer &operator=(const ElementBuffer&);
};

//!\brief Class managing an OpenGL uniform buffer object.
//!\note Should only be created when a GL context is active!
class UniformBuffer final : public BufferObject
{
public:
	explicit UniformBuffer(const void *data, size_t size, GLenum usage = GL_STREAM_DRAW);
	UniformBuffer(UniformBuffer &&tmp);
	virtual ~UniformBuffer() throw();

	void bind();
	void unbind();

	void bindRange(GLuint index);
private:
	UniformBuffer(const UniformBuffer&);
	UniformBuffer &operator=(const UniformBuffer&);
};

//!\brief Class describing structure of a uniform block containing useful
//!       information about the current OpenGL camera.
struct CameraBlock {
	Eigen::Matrix4f worldToClip;
	Eigen::Vector4f cameraPos;
	Eigen::Vector4f cameraDir;
};

class CameraBlockBuffer final
{
public:
	CameraBlockBuffer();
	~CameraBlockBuffer();

	void update();

	void bind();
	void unbind();

	void bindRange(GLuint index);

	CameraBlock block;
private:
	CameraBlockBuffer(const CameraBlockBuffer&);
	CameraBlockBuffer &operator=(const CameraBlockBuffer&);
	UniformBuffer buffer_;
};

class ShaderStorageBuffer final : public BufferObject
{
public:
	ShaderStorageBuffer(size_t sizeBytes, GLenum usage = GL_DYNAMIC_DRAW);
	ShaderStorageBuffer(ShaderStorageBuffer &&tmp);
	virtual ~ShaderStorageBuffer() throw();

	void bindBase(GLuint index);
private:
	ShaderStorageBuffer(const ShaderStorageBuffer&);
	ShaderStorageBuffer &operator=(const ShaderStorageBuffer&);
};

class AtomicCounterBuffer final : public BufferObject
{
public:
	AtomicCounterBuffer(GLenum usage = GL_DYNAMIC_DRAW);
	AtomicCounterBuffer(AtomicCounterBuffer &&tmp);
	virtual ~AtomicCounterBuffer() throw();

	void bindBase(GLuint index);
private:
	AtomicCounterBuffer(const AtomicCounterBuffer&);
	AtomicCounterBuffer &operator=(const AtomicCounterBuffer&);
};

}


#include "GLBuffer.hpp"

namespace glhelper {

BufferObject::BufferObject(size_t sizeBytes, GLenum usage, const void * data)
	:sizeBytes_(sizeBytes)
{
	glGenBuffers(1, &buf_);
	glBindBuffer(GL_ARRAY_BUFFER, buf_);
	glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

BufferObject::BufferObject(BufferObject && tmp)
{
	buf_ = tmp.buf_;
	sizeBytes_ = tmp.sizeBytes_;
	tmp.sizeBytes_ = 0;
}

BufferObject::~BufferObject() throw()
{
	if (sizeBytes_ != 0) {
		glDeleteBuffers(1, &buf_);
	}
}

void BufferObject::update(const void * data)
{
	update(data, sizeBytes_);
}

void BufferObject::update(const void * data, size_t sizeBytes)
{
	glBindBuffer(GL_ARRAY_BUFFER, buf_);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeBytes, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BufferObject::getData(void * data)
{
	getData(data, sizeBytes_);
}

void BufferObject::getData(void * data, size_t bytesToGet)
{
	glBindBuffer(GL_ARRAY_BUFFER, buf_);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, bytesToGet, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BufferObject::getData(void * data, size_t offset, size_t bytesToGet)
{
	glBindBuffer(GL_ARRAY_BUFFER, buf_);
	glGetBufferSubData(GL_ARRAY_BUFFER, offset, bytesToGet, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

size_t BufferObject::sizeBytes() const
{
	return sizeBytes_;
}

VertexBuffer::VertexBuffer(const std::vector<Eigen::Vector4f> &v, GLenum usage)
	:BufferObject(v.size()*sizeof(Eigen::Vector4f), usage, v.data())
{}

VertexBuffer::VertexBuffer(const std::vector<Eigen::Vector3f> &v, GLenum usage)
	:BufferObject(v.size()*sizeof(Eigen::Vector3f), usage, v.data())
{}

VertexBuffer::VertexBuffer(const std::vector<Eigen::Vector2f> &v, GLenum usage)
	:BufferObject(v.size()*sizeof(Eigen::Vector2f), usage, v.data())
{}

VertexBuffer::VertexBuffer(const std::vector<Eigen::Vector4i> &v, GLenum usage)
	:BufferObject(v.size()*sizeof(Eigen::Vector4i), usage, v.data())
{}

VertexBuffer::VertexBuffer(const std::vector<float> &v, GLenum usage)
	:BufferObject(v.size()*sizeof(float), usage, v.data())
{}

VertexBuffer::VertexBuffer(const size_t size, GLenum usage)
	:BufferObject(size, usage)

{}

VertexBuffer::VertexBuffer(VertexBuffer && tmp)
	:BufferObject(std::move(tmp))
{}

VertexBuffer::~VertexBuffer() throw()
{}

void VertexBuffer::bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, get());
}

void VertexBuffer::unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

ElementBuffer::ElementBuffer(const std::vector<GLuint> &v, GLenum usage)
	:BufferObject(v.size()*sizeof(GLuint), usage, v.data())
{}

ElementBuffer::ElementBuffer(const std::vector<GLushort> &v, GLenum usage)
	:BufferObject(v.size()*sizeof(GLushort), usage, v.data())
{}

ElementBuffer::ElementBuffer(size_t size, GLenum usage)
	:BufferObject(size, usage, nullptr)
{}

ElementBuffer::ElementBuffer(ElementBuffer && tmp)
	:BufferObject(std::move(tmp))
{}

ElementBuffer::~ElementBuffer() throw()
{}

void ElementBuffer::bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, get());
}

void ElementBuffer::unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

UniformBuffer::UniformBuffer(const void *data, size_t size, GLenum usage)
	:BufferObject(size, usage, data)
{}

UniformBuffer::UniformBuffer(UniformBuffer && tmp)
	:BufferObject(std::move(tmp))
{}

UniformBuffer::~UniformBuffer() throw()
{}

void UniformBuffer::bind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, get());
}

void UniformBuffer::unbind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::bindRange(GLuint index)
{
	glBindBufferRange(GL_UNIFORM_BUFFER, index, get(), 0, sizeBytes());
}

CameraBlockBuffer::CameraBlockBuffer()
:block(),
buffer_(&block, sizeof(CameraBlock))
{}

CameraBlockBuffer::~CameraBlockBuffer()
{}

void CameraBlockBuffer::update()
{
	buffer_.update(&block);
}
void CameraBlockBuffer::bind()
{
	buffer_.bind();
}
void CameraBlockBuffer::unbind()
{
	buffer_.unbind();
}
void CameraBlockBuffer::bindRange(GLuint index)
{
	buffer_.bindRange(index);
}

ShaderStorageBuffer::ShaderStorageBuffer(size_t sizeBytes, GLenum usage)
	:BufferObject(sizeBytes, usage)
{}

ShaderStorageBuffer::ShaderStorageBuffer(ShaderStorageBuffer && tmp)
	:BufferObject(std::move(tmp))
{}

ShaderStorageBuffer::~ShaderStorageBuffer()
{}

void ShaderStorageBuffer::bindBase(GLuint index)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, get());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, get());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}



AtomicCounterBuffer::AtomicCounterBuffer(GLenum usage)
	:BufferObject(sizeof(GLuint), usage)
{}

AtomicCounterBuffer::AtomicCounterBuffer(AtomicCounterBuffer && tmp)
	:BufferObject(std::move(tmp))
{}

AtomicCounterBuffer::~AtomicCounterBuffer() throw()
{}

void AtomicCounterBuffer::bindBase(GLuint index)
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, get());
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, index, get());
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

}

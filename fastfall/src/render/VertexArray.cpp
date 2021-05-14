#include "fastfall/render/VertexArray.hpp"

#include "GL/glew.h"
#include "detail/error.hpp"

namespace ff {

VertexArray::VertexArray(Primitive primitive_type, size_t size, VertexUsage usage)
	: m_primitive{ primitive_type },
	m_usage{ usage },
	m_vec{ size }
{
	glInitObjects();
}

VertexArray::VertexArray(const VertexArray& varray) {

	glInitObjects();
	m_usage = varray.m_usage;
	m_primitive = varray.m_primitive;
	m_vec = varray.m_vec;
	glTransfer();
}
VertexArray& VertexArray::operator= (const VertexArray& varray) {
	glInitObjects();
	m_usage = varray.m_usage;
	m_primitive = varray.m_primitive;
	m_vec = varray.m_vec;
	glTransfer();

	return *this;
}

VertexArray::VertexArray(VertexArray&& varray) noexcept {
	swap(varray);
}

VertexArray& VertexArray::operator= (VertexArray&& varray) noexcept {
	swap(varray);
	return *this;
}

void VertexArray::swap(VertexArray& varray) {
	std::swap(m_buffer, varray.m_buffer);
	std::swap(m_array, varray.m_array);

	m_usage = varray.m_usage;
	m_primitive = varray.m_primitive;

	std::swap(m_vec, varray.m_vec);
}

VertexArray::~VertexArray() {
	glCheck(glDeleteVertexArrays(1, &m_array));
	glCheck(glDeleteBuffers(1, &m_buffer));
}

void VertexArray::glTransfer() {
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));
	if (!m_bufferBound || m_vec.capacity() > m_bufferSize) {
		glCheck(glBufferData(GL_ARRAY_BUFFER, m_vec.capacity() * sizeof(Vertex), &m_vec[0], static_cast<GLenum>(m_usage)));
		m_bufferSize = m_vec.capacity();
		m_bufferBound = true;
	}
	else {
		glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0 * sizeof(Vertex), m_vec.size() * sizeof(Vertex), &m_vec[0]));
	}
}

void VertexArray::glTransfer(size_t startNdx, size_t count) {
	glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, startNdx * sizeof(Vertex), count * sizeof(Vertex), &m_vec[startNdx]);
}

void VertexArray::glInitObjects() {
	glCheck(glGenVertexArrays(1, &m_array));
	glCheck(glGenBuffers(1, &m_buffer));

	glCheck(glBindVertexArray(m_array));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));
	glCheck(glBufferData(GL_ARRAY_BUFFER, m_vec.capacity() * sizeof(Vertex), NULL, static_cast<GLenum>(m_usage)));
	m_bufferSize = m_vec.capacity();

	size_t position = 0lu;

	// position attribute
	glCheck(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ff::Vertex), (void*)position));
	glCheck(glEnableVertexAttribArray(0));
	position += (2 * sizeof(float));

	// color attribute
	glCheck(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ff::Vertex), (void*)position));
	glCheck(glEnableVertexAttribArray(1));
	position += sizeof(ff::Color);

	// tex attribute
	glCheck(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ff::Vertex), (void*)position));
	glCheck(glEnableVertexAttribArray(2));
}

void swap(VertexArray& lhs, VertexArray& rhs) {
	lhs.swap(rhs);
}

}
#include "fastfall/render/VertexArray.hpp"

#include "GL/glew.h"
#include "detail/error.hpp"
#include "fastfall/render.hpp"

namespace ff {

VertexArray::VertexArray(Primitive primitive_type, size_t size, VertexUsage usage)
	: m_primitive{ primitive_type },
	m_usage{ usage },
	m_vec{ size }
{

}

VertexArray::VertexArray(const VertexArray& varray) {

	m_usage = varray.m_usage;
	m_primitive = varray.m_primitive;
	m_vec = varray.m_vec;
}
VertexArray& VertexArray::operator= (const VertexArray& varray) {

	m_usage = varray.m_usage;
	m_primitive = varray.m_primitive;
	m_vec = varray.m_vec;

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
	std::swap(gl, varray.gl);
	std::swap(gl, varray.gl);

	m_usage = varray.m_usage;
	m_primitive = varray.m_primitive;

	std::swap(m_vec, varray.m_vec);
}

VertexArray::~VertexArray() {
	ff::glStaleVertexArrays(gl.m_array);
	ff::glStaleVertexBuffers(gl.m_buffer);
}

void VertexArray::glTransfer() const {

	if (gl.m_array == 0) {

		// do the opengl initializaion
		glCheck(glGenVertexArrays(1, &gl.m_array));
		glCheck(glGenBuffers(1, &gl.m_buffer));

		glCheck(glBindVertexArray(gl.m_array));
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, gl.m_buffer));
		glCheck(glBufferData(GL_ARRAY_BUFFER, m_vec.size() * sizeof(Vertex), NULL, static_cast<GLenum>(m_usage)));
		gl.m_bufsize = m_vec.size();

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

		if (gl.m_array == 0 || gl.m_buffer == 0) {
			LOG_ERR_("Unable to initialize vertex array for opengl");
			assert(false);
		}
	}

	if (!gl.sync) {

		// bind the buffer and transfer the vertex data to the video ram
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, gl.m_buffer));
		if (!gl.m_bound || m_vec.size() > gl.m_bufsize) {
			glCheck(glBufferData(GL_ARRAY_BUFFER, m_vec.size() * sizeof(Vertex), &m_vec[0], static_cast<GLenum>(m_usage)));
			gl.m_bufsize = m_vec.size();
			gl.m_bound = true;
		}
		else {			
			glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0 * sizeof(Vertex), m_vec.size() * sizeof(Vertex), &m_vec[0]));
		}
		gl.sync = true;
	}
}

void VertexArray::insert(size_t ndx, size_t count, Vertex value) {
	m_vec.insert(m_vec.cbegin() + ndx, count, value);
	gl.sync = false;
}

void VertexArray::erase(size_t ndx, size_t count) {
	m_vec.erase(m_vec.cbegin() + ndx, m_vec.cbegin() + ndx + count);
	gl.sync = false;
}

void VertexArray::draw(RenderTarget& target, RenderState state) const {
	target.draw(*this, state);
}

}
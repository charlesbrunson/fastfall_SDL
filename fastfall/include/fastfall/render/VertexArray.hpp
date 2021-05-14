#pragma once

#include "Vertex.hpp"
#include "Drawable.hpp"

#include "Transformable.hpp"
#include "Primitives.hpp"
#include "Texture.hpp"

#include <memory>

namespace ff {

class RenderTarget;

enum class VertexUsage {
	STATIC = GL_STATIC_DRAW,
	DYNAMIC = GL_DYNAMIC_DRAW,
	STREAM = GL_STREAM_DRAW,
};

class VertexArray : public Transformable {
public:

	VertexArray(Primitive primitive_type, size_t size = 0, VertexUsage usage = VertexUsage::DYNAMIC);
	~VertexArray();

	VertexArray(const VertexArray& varray);
	VertexArray& operator= (const VertexArray& varray);

	VertexArray(VertexArray&& varray) noexcept;
	VertexArray& operator= (VertexArray&& varray) noexcept;

	void swap(VertexArray& varray);

	void glTransfer();
	void glTransfer(size_t startNdx, size_t count);

	inline size_t size() const { return m_vec.size(); };
	inline bool empty() const { return m_vec.empty(); };

	inline void clear() {
		m_vec.clear();
	}

	inline std::vector<Vertex>& vec() { return m_vec; };

	const Vertex& operator[] (size_t ndx) const {
		return m_vec[ndx];
	}
	Vertex& operator[] (size_t ndx) {
		return m_vec[ndx];
	}

private:
	friend class RenderTarget;


	GLuint m_buffer = 0;
	GLuint m_array = 0;

	bool m_bufferBound = false;
	size_t m_bufferSize = 0;

	VertexUsage m_usage;
	Primitive m_primitive;

	std::vector<Vertex> m_vec;

	void glInitObjects();
};

void swap(VertexArray& lhs, VertexArray& rhs);

}
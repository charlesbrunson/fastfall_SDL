#pragma once

#include "fastfall/render/util/Vertex.hpp"
#include "Drawable.hpp"

#include "fastfall/render/util/Transformable.hpp"
#include "fastfall/render/util/Primitives.hpp"
#include "fastfall/render/util/Texture.hpp"

#include <memory>
#include <vector>

namespace ff {

class RenderTarget;

enum class VertexUsage {
	STATIC = GL_STATIC_DRAW,
	DYNAMIC = GL_DYNAMIC_DRAW,
	STREAM = GL_STREAM_DRAW,
};

class VertexArray : public Transformable, public Drawable {
private:

	// used to synchronize the state on the vram prior to drawing
	// mutable because i am not smart
	struct GPUState {
		GLuint m_array = 0;
		GLuint m_buffer = 0;

		size_t m_bufsize = 0;
		bool m_bound = false;

		bool sync = false;
	} mutable gl;


public:

	VertexArray(Primitive primitive_type, size_t size = 0, VertexUsage usage = VertexUsage::DYNAMIC);
	~VertexArray();

	VertexArray(const VertexArray& varray);
	VertexArray& operator= (const VertexArray& varray);

	VertexArray(VertexArray&& varray) noexcept;
	VertexArray& operator= (VertexArray&& varray) noexcept;

	void swap(VertexArray& varray);

	inline size_t size() const { return m_vec.size(); };
	inline bool empty() const { return m_vec.empty(); };


	void insert(size_t ndx, size_t count = 1, Vertex value = Vertex{});
	void erase(size_t ndx, size_t count = 1);

	inline void clear() {
		m_vec.clear();
	}

	inline const std::vector<Vertex>& vec() const { 
		return m_vec; 
	};

	const Vertex& operator[] (size_t ndx) const {
		return m_vec[ndx];
	}
	Vertex& operator[] (size_t ndx) {
		gl.sync = false;
		return m_vec[ndx];
	}

private:
	friend class RenderTarget;
	void glTransfer() const;

	VertexUsage m_usage;
	Primitive m_primitive;

	void draw(RenderTarget& target, RenderState state = RenderState()) const override;


	std::vector<Vertex> m_vec;

};

}
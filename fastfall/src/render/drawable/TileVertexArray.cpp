#include "fastfall/render/drawable/TileVertexArray.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/util/Vec2.hpp"

#include "fastfall/engine/config.hpp"
#include "fastfall/render/target/RenderTarget.hpp"

#include <assert.h>
#include <iostream>

namespace ff {

constexpr unsigned VERTICES_PER_TILE = 6;

TileVertexArray::TileVertexArray()
	: m_verts(ff::Primitive::TRIANGLES)
{
	m_tex = TextureRef{};
	m_tile_count = 0;
}

TileVertexArray::TileVertexArray(Vec2u arr_size)
	: m_verts(ff::Primitive::TRIANGLES, (size_t)arr_size.x * arr_size.y * VERTICES_PER_TILE)
	, m_has_tile((size_t)arr_size.x * arr_size.y, false)
{
	m_tex = TextureRef{};
	m_size = arr_size;
	m_tile_count = 0;
}

void TileVertexArray::setTexture(const Texture& texture) noexcept {
	m_tex = texture;
}

void TileVertexArray::resize(Vec2u size)
{
	m_size = size;
	m_verts = VertexArray{ ff::Primitive::TRIANGLES, (size_t)m_size.x * m_size.y * VERTICES_PER_TILE };
	m_has_tile = std::vector<bool>((size_t)m_size.x * m_size.y, false);
	m_tile_count = 0;
}

const TextureRef& TileVertexArray::getTexture() const noexcept {
	return m_tex;
}

void TileVertexArray::setTile(Vec2u at, Vec2u texPos) {
	assert(at.x <= m_size.x && at.y <= m_size.y);

	constexpr std::array<glm::fvec2, 6> offsets{
		glm::fvec2(0.f, 0.f),
		glm::fvec2(1.f, 0.f),
		glm::fvec2(0.f, 1.f),
		glm::fvec2(1.f, 1.f),
		glm::fvec2(0.f, 1.f),
		glm::fvec2(1.f, 0.f)
	};

	// terrible awful no good uv mapping hack
	constexpr float uv_bias = 1.f / 16384.f;
	constexpr std::array<glm::fvec2, 6> bias{
		glm::fvec2( uv_bias,  uv_bias),
		glm::fvec2(-uv_bias,  uv_bias),
		glm::fvec2( uv_bias, -uv_bias),
		glm::fvec2(-uv_bias, -uv_bias),
		glm::fvec2( uv_bias, -uv_bias),
		glm::fvec2(-uv_bias,  uv_bias)
	};


	auto pos = glm::fvec2(at.x, at.y);
	auto texpos = glm::fvec2(texPos.x, texPos.y);

	size_t vndx = (size_t)(at.x) + ((size_t)at.y * m_size.x);

	if (!m_has_tile[vndx]) { m_tile_count++; }
	m_has_tile[vndx] = true;

	vndx *= VERTICES_PER_TILE;

	for (int i = 0; i < VERTICES_PER_TILE; i++) {
		m_verts[vndx + i].color = Color::White;
		m_verts[vndx + i].pos = (pos + offsets[i]) * TILESIZE_F;

		glm::fvec2 tilesize {m_tex.get()->inverseSize() * TILESIZE_F};
		m_verts[vndx + i].tex_pos = ((texpos + offsets[i]) * tilesize) + bias[i];
	}

}

void TileVertexArray::blank(Vec2u at) {
	assert(at.x <= m_size.x && at.y <= m_size.y);

	size_t vndx = (size_t)(at.x) + ((size_t)at.y * m_size.x);

	if (m_has_tile[vndx]) { m_tile_count--; }
	m_has_tile[vndx] = false;

	vndx *= VERTICES_PER_TILE;
	memset(&m_verts[vndx], 0, sizeof(Vertex) * VERTICES_PER_TILE);
	
	/*
	for (int i = 0; i < VERTICES_PER_TILE; i++) {
		m_verts[vndx + i].color = Color::Transparent;
		m_verts[vndx + i].pos = glm::fvec2{ 0, 0 };
		m_verts[vndx + i].tex_pos = glm::fvec2{ 0, 0 };
	}
	*/
}


void TileVertexArray::clear() {
	m_verts = VertexArray{ ff::Primitive::TRIANGLES, (size_t)m_size.x * m_size.y * VERTICES_PER_TILE };
}

void TileVertexArray::draw(RenderTarget& target, RenderState states) const {
	LOG_INFO("tiles={}", m_tile_count);

	if (m_verts.empty() || m_tile_count == 0)
		return;

	states.transform = Transform::combine(states.transform, Transform(offset));
	states.texture = m_tex;

	target.draw(m_verts, states);
}

}

#include "fastfall/render/TileVertexArray.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/util/Vec2.hpp"

#include "fastfall/render/RenderTarget.hpp"

#include <assert.h>
#include <iostream>

namespace ff {

constexpr unsigned VERTICES_PER_TILE = 6;

TileVertexArray::TileVertexArray()
	: m_verts(ff::Primitive::TRIANGLES)
{
	m_tex = TextureRef{};
}

TileVertexArray::TileVertexArray(Vec2u arr_size)
	: m_verts(ff::Primitive::TRIANGLES)
{
	m_tex = TextureRef{};
	m_size = arr_size;

	m_tiles.reserve((size_t)m_size.x * m_size.y);
}

void TileVertexArray::setTexture(const Texture& texture) noexcept {
	m_tex = texture;
}

void TileVertexArray::resize(Vec2u size)
{
	m_size = size;
	m_tiles.reserve((size_t)m_size.x * m_size.y);
}

const TextureRef& TileVertexArray::getTexture() const noexcept {
	return m_tex;
}

void TileVertexArray::setTile(Vec2u at, Vec2u texPos) {
	assert(at.x <= m_size.x && at.y <= m_size.y);

	auto tile = std::upper_bound(m_tiles.begin(), m_tiles.end(), Tile{ at, texPos },
		[](const Tile& lhs, const Tile& rhs) {
			return lhs.position <= rhs.position;
		});

	size_t vndx;
	if (tile == m_tiles.end()) {
		m_tiles.resize(m_tiles.size() + 1);
		tile = m_tiles.end() - 1;

		m_verts.insert(m_verts.size(), VERTICES_PER_TILE);

		vndx = m_verts.size() - VERTICES_PER_TILE;
	}
	else if (tile->position != at) {
		tile = m_tiles.insert(tile, Tile());

		ptrdiff_t distance = (std::distance(m_tiles.begin(), tile) * VERTICES_PER_TILE);

		m_verts.insert(distance, VERTICES_PER_TILE);

		vndx = distance;
	}
	else {
		vndx = std::distance(m_tiles.begin(), tile) * VERTICES_PER_TILE;
	}
	
	tile->position = at;
	tile->tex_position = texPos;

	auto pos = glm::fvec2(at.x, at.y);
	auto texpos = glm::fvec2(texPos.x, texPos.y);

	constexpr std::array<glm::fvec2, 6> offsets{
		glm::fvec2(0.f, 0.f),
		glm::fvec2(1.f, 0.f),
		glm::fvec2(0.f, 1.f),
		glm::fvec2(1.f, 1.f),
		glm::fvec2(0.f, 1.f),
		glm::fvec2(1.f, 0.f)
	};

	for (int i = 0; i < VERTICES_PER_TILE; i++) {
		m_verts[vndx + i].color = Color::White;
		m_verts[vndx + i].pos = (pos + offsets[i]) * TILESIZE_F;
		m_verts[vndx + i].tex_pos = (texpos + offsets[i]) * TILESIZE_F * m_tex.get()->inverseSize();
	}
}

void TileVertexArray::erase(Vec2u at) {
	auto iter = std::find_if(m_tiles.begin(), m_tiles.end(),
		[&at](const Tile& t) {
			return t.position == at;
		}
	);

	if (iter != m_tiles.end()) {
		int ndx = std::distance(m_tiles.begin(), iter) * VERTICES_PER_TILE;
		m_tiles.erase(iter);
		m_verts.erase(ndx, VERTICES_PER_TILE);
	}
}

void TileVertexArray::blank(Vec2u at) {

	auto iter = std::find_if(m_tiles.begin(), m_tiles.end(),
		[&at](const Tile& t) {
			return t.position == at;
		}
	);

	if (iter != m_tiles.end()) {
		size_t ndx = std::distance(m_tiles.begin(), iter) * VERTICES_PER_TILE;

		for (int i = 0; i < VERTICES_PER_TILE; i++) {
			m_verts[ndx + i].color.a = 0;
		}
	}
}

void TileVertexArray::clear() {
	m_tiles.clear();
	m_verts.clear();
}

void TileVertexArray::draw(RenderTarget& target, RenderState states) const {
	if (m_verts.empty())
		return;

	states.transform = Transform::combine(states.transform, Transform(offset));
	states.texture = m_tex;

	target.draw(m_verts, states);
}

}
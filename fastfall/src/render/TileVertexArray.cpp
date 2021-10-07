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
	: m_verts(ff::Primitive::TRIANGLES, (size_t)arr_size.x * arr_size.y * VERTICES_PER_TILE)
{
	m_tex = TextureRef{};
	m_size = arr_size;
}

void TileVertexArray::setTexture(const Texture& texture) noexcept {
	m_tex = texture;
}

void TileVertexArray::resize(Vec2u size)
{
	m_size = size;
	m_verts = VertexArray{ ff::Primitive::TRIANGLES, (size_t)m_size.x * m_size.y * VERTICES_PER_TILE };
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

	auto pos = glm::fvec2(at.x, at.y);
	auto texpos = glm::fvec2(texPos.x, texPos.y);

	size_t vndx = (size_t)(at.x) + (size_t)(at.y * m_size.x);
	vndx *= VERTICES_PER_TILE;

	for (int i = 0; i < VERTICES_PER_TILE; i++) {
		m_verts[vndx + i].color = Color::White;
		m_verts[vndx + i].pos = (pos + offsets[i]) * TILESIZE_F;
		m_verts[vndx + i].tex_pos = (texpos + offsets[i]) * TILESIZE_F * m_tex.get()->inverseSize();
	}
}

void TileVertexArray::erase(Vec2u at) {
	assert(at.x <= m_size.x && at.y <= m_size.y);

	size_t vndx = (size_t)(at.x) + (size_t)(at.y * m_size.x);
	vndx *= VERTICES_PER_TILE;

	for (int i = 0; i < VERTICES_PER_TILE; i++) {
		m_verts[vndx + i].color = Color::Transparent;
		m_verts[vndx + i].tex_pos = glm::fvec2{ 0, 0 };
	}
}

void TileVertexArray::blank(Vec2u at) {
	assert(at.x <= m_size.x && at.y <= m_size.y);

	size_t vndx = (size_t)(at.x) + (size_t)(at.y * m_size.x);
	vndx *= VERTICES_PER_TILE;

	for (int i = 0; i < VERTICES_PER_TILE; i++) {
		m_verts[vndx + i].color = Color::Transparent;
	}
}

void TileVertexArray::clear() {
	m_verts = VertexArray{ ff::Primitive::TRIANGLES, (size_t)m_size.x * m_size.y * VERTICES_PER_TILE };
}

void TileVertexArray::draw(RenderTarget& target, RenderState states) const {
	if (m_verts.empty())
		return;

	states.transform = Transform::combine(states.transform, Transform(offset));
	states.texture = m_tex;

	target.draw(m_verts, states);
}

}
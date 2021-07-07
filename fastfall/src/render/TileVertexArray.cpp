#include "fastfall/render/TileVertexArray.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/util/Vec2.hpp"

#include "fastfall/render/RenderTarget.hpp"

#include <assert.h>
#include <iostream>

namespace ff {

constexpr unsigned VERTICES_PER_TILE = 6;

TileVertexArray::TileVertexArray()
	: verts(ff::Primitive::TRIANGLES) 
{
	tex = TextureRef{};
}
TileVertexArray::TileVertexArray(Vec2u arr_size)
	: verts(ff::Primitive::TRIANGLES)
{
	tex = TextureRef{};
	size = arr_size;

	tiles.reserve((size_t)size.x * size.y);
}

TileVertexArray::~TileVertexArray() {

}

void TileVertexArray::setTexture(const Texture& texture) noexcept {
	tex = texture;
}

const TextureRef& TileVertexArray::getTexture() const noexcept {
	return tex;
}
void TileVertexArray::setTile(Vec2u at, Vec2u texPos) {
	assert(at.x <= size.x && at.y <= size.y);

	auto tile = std::upper_bound(tiles.begin(), tiles.end(), Tile{ at, texPos },
		[](const Tile& lhs, const Tile& rhs) {
			return lhs.position <= rhs.position;
		});

	size_t vndx;
	if (tile == tiles.end()) {
		tiles.resize(tiles.size() + 1);
		tile = tiles.end() - 1;

		verts.insert(verts.size(), VERTICES_PER_TILE);

		vndx = verts.size() - VERTICES_PER_TILE;
	}
	else if (tile->position != at) {
		tile = tiles.insert(tile, Tile());

		ptrdiff_t distance = (std::distance(tiles.begin(), tile) * VERTICES_PER_TILE);

		verts.insert(distance, VERTICES_PER_TILE);

		vndx = distance;
	}
	else {
		vndx = std::distance(tiles.begin(), tile) * VERTICES_PER_TILE;
	}
	
	tile->position = at;
	tile->tex_position = texPos;

	auto pos = glm::fvec2(at.x, at.y);
	auto texpos = glm::fvec2(texPos.x, texPos.y);

	/*
	pos += glm::fvec2(rotation_offset.x, rotation_offset.y);
	if (pos.x >= size.x) pos.x -= size.x;
	if (pos.y >= size.y) pos.y -= size.y;
	*/

	constexpr std::array<glm::fvec2, 6> offsets{
		glm::fvec2(0.f, 0.f),
		glm::fvec2(1.f, 0.f),
		glm::fvec2(0.f, 1.f),
		glm::fvec2(1.f, 1.f),
		glm::fvec2(0.f, 1.f),
		glm::fvec2(1.f, 0.f)
	};

	for (int i = 0; i < VERTICES_PER_TILE; i++) {
		verts[vndx + i].color = Color::White;
		verts[vndx + i].pos = (pos + offsets[i]) * TILESIZE_F;
		verts[vndx + i].tex_pos = (texpos + offsets[i]) * TILESIZE_F * tex.get()->inverseSize();
	}
}


void TileVertexArray::erase(Vec2u at) {
	auto iter = std::find_if(tiles.begin(), tiles.end(),
		[&at](const Tile& t) {
			return t.position == at;
		}
	);

	if (iter != tiles.end()) {
		int ndx = std::distance(tiles.begin(), iter) * VERTICES_PER_TILE;
		tiles.erase(iter);
		verts.erase(ndx, VERTICES_PER_TILE);
	}
}
void TileVertexArray::blank(Vec2u at) {

	auto iter = std::find_if(tiles.begin(), tiles.end(),
		[&at](const Tile& t) {
			return t.position == at;
		}
	);

	if (iter != tiles.end()) {
		size_t ndx = std::distance(tiles.begin(), iter) * VERTICES_PER_TILE;
		
		for (int i = 0; i < VERTICES_PER_TILE; i++) {
			verts[ndx + i].color.a = 0;
		}
	}
}

void TileVertexArray::clear() {
	tiles.clear();
	verts.clear();
}

void TileVertexArray::draw(RenderTarget& target, RenderState states) const {
	if (verts.empty())
		return;

	states.transform = Transform::combine(states.transform, Transform(offset));
	states.texture = tex;

	target.draw(verts, states);
}

}
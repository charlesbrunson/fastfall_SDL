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

	pos += glm::fvec2(rotation_offset.x, rotation_offset.y);
	if (pos.x >= size.x) pos.x -= size.x;
	if (pos.y >= size.y) pos.y -= size.y;

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

		//verts.vec().erase(verts.vec().begin() + ndx, verts.vec().begin() + ndx + VERTICES_PER_TILE);
	}
}
void TileVertexArray::blank(Vec2u at) {

	auto iter = std::find_if(tiles.begin(), tiles.end(),
		[&at](const Tile& t) {
			return t.position == at;
		}
	);

	//Vertex* v_ptr;
	if (iter != tiles.end()) {
		size_t ndx = std::distance(tiles.begin(), iter) * VERTICES_PER_TILE;
		//v_ptr = &verts[ndx];
		
		for (int i = 0; i < VERTICES_PER_TILE; i++) {
			verts[ndx + i].color.a = 0;
		}
	}
}

void TileVertexArray::clear() {
	tiles.clear();
	verts.clear();
}

void TileVertexArray::rotate_forwardX() {
	if (size.x <= 1)
		return;

	rotation_offset.x++;
	rotation_offset.x %= size.x;

	float end = (size.x - 1) * TILESIZE_F;

	//Vertex* v_ptr;
	for (size_t i = 0; i < verts.size(); i += VERTICES_PER_TILE) {
		//v_ptr = &verts[i];

		if (verts[i].pos.x == end) [[unlikely]]
		{
			for (size_t j = 0; j < VERTICES_PER_TILE; j++) {
				verts[i + j].pos.x -= end;
			}
		}
		else
		{
			for (size_t j = 0; j < VERTICES_PER_TILE; j++) {
				verts[i + j].pos.x += TILESIZE_F;
			}
		}
	}
}

void TileVertexArray::rotate_backwardX() {
	if (size.x <= 1)
		return;

	if (rotation_offset.x == 0) {
		rotation_offset.x = size.x - 1;
	}
	else {
		rotation_offset.x--;
	}

	float end = (size.x - 1) * TILESIZE_F;

	//Vertex* v_ptr;
	for (size_t i = 0; i < verts.size(); i += VERTICES_PER_TILE) {
		//v_ptr = &verts[i];

		if (verts[i].pos.x == 0) [[unlikely]]
		{
			for (size_t j = 0; j < VERTICES_PER_TILE; j++) {
				verts[i + j].pos.x += end;
			}
		}
		else
		{
			for (size_t j = 0; j < VERTICES_PER_TILE; j++) {
				verts[i + j].pos.x -= TILESIZE_F;
			}
		}
	}


}
void TileVertexArray::rotate_forwardY() {
	if (size.y <= 1)
		return;

	rotation_offset.y++;
	rotation_offset.y %= size.y;

	float end = (size.y - 1) * TILESIZE_F;

	//Vertex* v_ptr;
	for (size_t i = 0; i < verts.size(); i += VERTICES_PER_TILE) {
		//v_ptr = &verts[i];

		if (verts[i].pos.y == end) [[unlikely]]
		{
			for (size_t j = 0; j < VERTICES_PER_TILE; j++) {
				verts[i + j].pos.y -= end;
			}
		}
		else
		{
			for (size_t j = 0; j < VERTICES_PER_TILE; j++) {
				verts[i + j].pos.y += TILESIZE_F;
			}
		}
	}
}
void TileVertexArray::rotate_backwardY() {
	if (size.y <= 1)
		return;

	if (rotation_offset.y == 0) {
		rotation_offset.y = size.y - 1;
	}
	else {
		rotation_offset.y--;
	}

	float end = (size.y - 1) * TILESIZE_F;

	//Vertex* v_ptr;
	for (size_t i = 0; i < verts.size(); i += VERTICES_PER_TILE) {
		//v_ptr = &verts[i];

		if (verts[i].pos.y == 0) [[unlikely]]
		{
			for (size_t j = 0; j < VERTICES_PER_TILE; j++) {
				verts[i + j].pos.y += end;
			}
		}
		else
		{
			for (size_t j = 0; j < VERTICES_PER_TILE; j++) {
				verts[i + j].pos.y -= TILESIZE_F;
			}
		}
	}
}

void TileVertexArray::draw(RenderTarget& target, RenderState states) const {
	if (verts.empty())
		return;

	//RenderStates shift = states;
	states.transform = Transform::combine(states.transform, Transform(offset));
	states.texture = tex;

	target.draw(verts, states);
	//target.draw(&verts[0], verts.size(), sf::Quads, states);
}



}
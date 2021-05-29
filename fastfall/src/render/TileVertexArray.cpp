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
	verts.vec().reserve((size_t)size.x * size.y * VERTICES_PER_TILE);

}

TileVertexArray::~TileVertexArray() {

}

void TileVertexArray::setTexture(const Texture& texture) noexcept {
	tex = texture;
}
const TextureRef& TileVertexArray::getTexture() noexcept {
	return tex;
}
void TileVertexArray::setTile(Vec2u at, Vec2u texPos) {
	assert(at.x <= size.x && at.y <= size.y);

	auto tile = std::upper_bound(tiles.begin(), tiles.end(), Tile{ at, texPos },
		[](const Tile& lhs, const Tile& rhs) {
			return lhs.position <= rhs.position;
		});

	std::vector<Vertex>& vertvec = verts.vec();

	Vertex* v_ptr;

	{
		auto vert = vertvec.end();

		if (tile == tiles.end()) {
			tiles.resize(tiles.size() + 1);
			tile = tiles.end() - 1;

			//verts.resize(verts.size() + VERTICES_PER_TILE);
			vertvec.insert(vertvec.end(), VERTICES_PER_TILE, Vertex{});

			vert = vertvec.end() - VERTICES_PER_TILE;
		}
		else if (tile->position != at) {
			tile = tiles.insert(tile, Tile());

			ptrdiff_t distance = (std::distance(tiles.begin(), tile) * VERTICES_PER_TILE);

			vert = vertvec.insert(
				vertvec.begin() + distance,
				VERTICES_PER_TILE,
				Vertex{});

		}
		else {
			vert = vertvec.begin() + (std::distance(tiles.begin(), tile) * VERTICES_PER_TILE);
		}
		assert(vert != vertvec.end());

		v_ptr = &*vert;
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
		v_ptr[i].color = Color::White;
		v_ptr[i].pos = (pos + offsets[i]) * TILESIZE_F;
		v_ptr[i].tex_pos = (texpos + offsets[i]) * TILESIZE_F * tex.get()->inverseSize();
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
		verts.vec().erase(verts.vec().begin() + ndx, verts.vec().begin() + ndx + VERTICES_PER_TILE);
	}
}
void TileVertexArray::blank(Vec2u at) {

	auto iter = std::find_if(tiles.begin(), tiles.end(),
		[&at](const Tile& t) {
			return t.position == at;
		}
	);

	Vertex* v_ptr;
	if (iter != tiles.end()) {
		size_t ndx = std::distance(tiles.begin(), iter) * VERTICES_PER_TILE;
		v_ptr = &verts[ndx];
		
		for (int i = 0; i < VERTICES_PER_TILE; i++) {
			v_ptr[i].color.a = 0;
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

	Vertex* v_ptr;
	for (size_t i = 0; i < verts.size(); i += VERTICES_PER_TILE) {
		v_ptr = &verts[i];

		if (v_ptr[0].pos.x == end) [[unlikely]]
		{
			for (int i = 0; i < VERTICES_PER_TILE; i++) {
				v_ptr[i].pos.x -= end;
			}
		}
		else
		{
			for (int i = 0; i < VERTICES_PER_TILE; i++) {
				v_ptr[i].pos.x += TILESIZE_F;
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

	Vertex* v_ptr;
	for (size_t i = 0; i < verts.size(); i += VERTICES_PER_TILE) {
		v_ptr = &verts[i];

		if (v_ptr[0].pos.x == 0) [[unlikely]]
		{
			for (int i = 0; i < VERTICES_PER_TILE; i++) {
				v_ptr[i].pos.x += end;
			}
		}
		else
		{
			for (int i = 0; i < VERTICES_PER_TILE; i++) {
				v_ptr[i].pos.x -= TILESIZE_F;
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

	Vertex* v_ptr;
	for (size_t i = 0; i < verts.size(); i += VERTICES_PER_TILE) {
		v_ptr = &verts[i];

		if (v_ptr[0].pos.y == end) [[unlikely]]
		{
			for (int i = 0; i < VERTICES_PER_TILE; i++) {
				v_ptr[i].pos.y -= end;
			}
		}
		else
		{
			for (int i = 0; i < VERTICES_PER_TILE; i++) {
				v_ptr[i].pos.y += TILESIZE_F;
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

	Vertex* v_ptr;
	for (size_t i = 0; i < verts.size(); i += VERTICES_PER_TILE) {
		v_ptr = &verts[i];

		if (v_ptr[0].pos.y == 0) [[unlikely]]
		{
			for (int i = 0; i < VERTICES_PER_TILE; i++) {
				v_ptr[i].pos.y += end;
			}
		}
		else
		{
			for (int i = 0; i < VERTICES_PER_TILE; i++) {
				v_ptr[i].pos.y -= TILESIZE_F;
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
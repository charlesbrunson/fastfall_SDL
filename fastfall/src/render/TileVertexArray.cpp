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
	: verts(ff::Primitive::TRIANGLES, arr_size.x * arr_size.y * VERTICES_PER_TILE)
{
	tex = TextureRef{};
	size = arr_size;

	tiles.reserve(size.x * size.y);
	//verts.reserve(size.x * size.y * VERTICES_PER_TILE);
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
		vert = vertvec.begin() + (std::distance(tiles.begin(), tile) * 4);
	}
	assert(vert != vertvec.end());

	tile->position = at;
	tile->tex_position = texPos;

	constexpr auto topleft = glm::fvec2(0.f, 0.f);
	constexpr auto topright = glm::fvec2(1.f, 0.f);
	constexpr auto botleft = glm::fvec2(0.f, 1.f);
	constexpr auto botright = glm::fvec2(1.f, 1.f);

	auto pos = glm::fvec2(at.x, at.y);
	auto texpos = glm::fvec2(texPos.x, texPos.y);

	auto pos_topleft = (pos + topleft) * TILESIZE_F;
	auto pos_topright = (pos + topright) * TILESIZE_F;
	auto pos_botleft = (pos + botleft) * TILESIZE_F;
	auto pos_botright = (pos + botright) * TILESIZE_F;

	auto tex_topleft = (texpos + topleft) * TILESIZE_F;
	auto tex_topright = (texpos + topright) * TILESIZE_F;
	auto tex_botleft = (texpos + botleft) * TILESIZE_F;
	auto tex_botright = (texpos + botright) * TILESIZE_F;

	vert->color.a = 255;
	vert->pos = pos_topleft;
	vert->tex_pos = tex_topleft;
	vert++;

	vert->color.a = 255;
	vert->pos = pos_topright;
	vert->tex_pos = tex_topright;
	vert++;

	vert->color.a = 255;
	vert->pos = pos_botleft;
	vert->tex_pos = tex_botleft;
	vert++;

	vert->color.a = 255;
	vert->pos = pos_botright;
	vert->tex_pos = tex_botright;
	vert++;

	vert->color.a = 255;
	vert->pos = pos_botleft;
	vert->tex_pos = tex_botleft;
	vert++;

	vert->color.a = 255;
	vert->pos = pos_topright;
	vert->tex_pos = tex_topright;
	vert++;
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

	if (iter != tiles.end()) {
		size_t ndx = std::distance(tiles.begin(), iter) * VERTICES_PER_TILE;
		//tiles.erase(iter);

		verts[ndx + 0].color.a = 0;
		verts[ndx + 1].color.a = 0;
		verts[ndx + 2].color.a = 0;
		verts[ndx + 3].color.a = 0;
		verts[ndx + 4].color.a = 0;
		verts[ndx + 5].color.a = 0;

		//verts.erase(verts.begin() + ndx, verts.begin() + ndx + 4);
	}
}

void TileVertexArray::clear() {
	tiles.clear();
	verts.clear();
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
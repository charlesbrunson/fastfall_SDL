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
TileVertexArray::TileVertexArray(Vec2u arr_size, bool fillInit)
	: verts(ff::Primitive::TRIANGLES),
	filled(fillInit)
{
	tex = TextureRef{};
	size = arr_size;

	tiles.reserve((size_t)size.x * size.y);

	if (fillInit) {
		verts.vec().resize((size_t)size.x * size.y * VERTICES_PER_TILE, Vertex(glm::fvec2(), Color::Transparent, glm::fvec2()));
		for (size_t x = 0; x < size.x; x++) {
			for (size_t y = 0; y < size.y; y++) {
				verts[((size_t)y * size.x + x) * VERTICES_PER_TILE].pos = Vec2f{ (float)x, (float)y } * TILESIZE_F;
				verts[((size_t)y * size.x + x) * VERTICES_PER_TILE + 1].pos = Vec2f{ (float)x + 1, (float)y } * TILESIZE_F;
				verts[((size_t)y * size.x + x) * VERTICES_PER_TILE + 2].pos = Vec2f{ (float)x, (float)y + 1} * TILESIZE_F;
				verts[((size_t)y * size.x + x) * VERTICES_PER_TILE + 3].pos = Vec2f{ (float)x + 1, (float)y + 1 } * TILESIZE_F;
				verts[((size_t)y * size.x + x) * VERTICES_PER_TILE + 4].pos = Vec2f{ (float)x, (float)y + 1 } * TILESIZE_F;
				verts[((size_t)y * size.x + x) * VERTICES_PER_TILE + 5].pos = Vec2f{ (float)x + 1, (float)y } * TILESIZE_F;

			}
		}
	}
	else {
		verts.vec().reserve((size_t)size.x * size.y * VERTICES_PER_TILE);
	}
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

	//at = at + rotation_offset;
	//at.x %= size.x;
	//at.y %= size.y;

	auto tile = std::upper_bound(tiles.begin(), tiles.end(), Tile{ at, texPos },
		[](const Tile& lhs, const Tile& rhs) {
			return lhs.position <= rhs.position;
		});

	std::vector<Vertex>& vertvec = verts.vec();

	Vertex* v_ptr;

	if (!filled) {
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
	else {
		if (tile == tiles.end()) {
			tiles.resize(tiles.size() + 1);
			tile = tiles.end() - 1;
		}
		else if (tile->position != at) {
			tile = tiles.insert(tile, Tile());
		}
		v_ptr = &vertvec[((size_t)at.y * size.x + at.x) * VERTICES_PER_TILE];
	}


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

	auto tex_topleft = (texpos + topleft) * TILESIZE_F * tex.get()->inverseSize();
	auto tex_topright = (texpos + topright) * TILESIZE_F * tex.get()->inverseSize();
	auto tex_botleft = (texpos + botleft) * TILESIZE_F * tex.get()->inverseSize();
	auto tex_botright = (texpos + botright) * TILESIZE_F * tex.get()->inverseSize();

	v_ptr->color = Color::White;
	v_ptr->pos = pos_topleft;
	v_ptr->tex_pos = tex_topleft;
	v_ptr++;

	v_ptr->color = Color::White;
	v_ptr->pos = pos_topright;
	v_ptr->tex_pos = tex_topright;
	v_ptr++;

	v_ptr->color = Color::White;
	v_ptr->pos = pos_botleft;
	v_ptr->tex_pos = tex_botleft;
	v_ptr++;

	v_ptr->color = Color::White;
	v_ptr->pos = pos_botright;
	v_ptr->tex_pos = tex_botright;
	v_ptr++;

	v_ptr->color = Color::White;
	v_ptr->pos = pos_botleft;
	v_ptr->tex_pos = tex_botleft;
	v_ptr++;

	v_ptr->color = Color::White;
	v_ptr->pos = pos_topright;
	v_ptr->tex_pos = tex_topright;
	v_ptr++;
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

void TileVertexArray::rotate_forwardX() {
	if (size.x <= 1)
		return;

	rotation_offset.x++;
	rotation_offset.x %= size.x;

	size_t vwidth = (size_t)size.x * 6;

	auto at = [&vwidth](const size_t& x, const size_t& y) -> size_t {
		return y * vwidth + x * 6;
	};

	for (int y = 0; y < size.y; y++) {

		size_t ndx = y * vwidth;

		std::array<Vec2f, 6> texpos;
		std::array<Color, 6> col;
		for (int i = 0; i < 6; i++) {
			texpos[i] = verts[at(size.x - 1, y) + i].tex_pos;
			col[i] = verts[at(size.x - 1, y) + i].color;
		}
		for (int i = vwidth - 1; i >= 6; i--) {
			verts[at(0, y) + i].tex_pos = verts[at(0, y) + i - 6].tex_pos;
			verts[at(0, y) + i].color = verts[at(0, y) + i - 6].color;
		}
		for (int i = 0; i < 6; i++) {
			verts[at(0, y) + i].tex_pos = texpos[i];
			verts[at(0, y) + i].color = col[i];
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

	size_t vwidth = (size_t)size.x * 6;

	auto at = [&vwidth](const size_t& x, const size_t& y) -> size_t {
		return y * vwidth + x * 6;
	};

	for (int y = 0; y < size.y; y++) {

		std::array<Vec2f, 6> texpos;
		std::array<Color, 6> col;
		for (int i = 0; i < 6; i++) {
			texpos[i] = verts[at(0, y) + i].tex_pos;
			col[i] = verts[at(0, y) + i].color;
		}
		for (int i = 0; i < vwidth - 6; i++) {
			verts[at(0, y) + i].tex_pos = verts[at(0, y) + i + 6].tex_pos;
			verts[at(0, y) + i].color = verts[at(0, y) + i + 6].color;
		}
		for (int i = 0; i < 6; i++) {
			verts[at(size.x - 1, y) + i].tex_pos = texpos[i];
			verts[at(size.x - 1, y) + i].color = col[i];
		}
	}
}
void TileVertexArray::rotate_forwardY() {
	if (size.y <= 1)
		return;

	rotation_offset.y++;
	rotation_offset.y %= size.y;

	size_t vwidth = (size_t)size.x * 6;
	size_t vheight = (size_t)size.y * 6;

	auto at = [&vwidth](const size_t& x, const size_t& y) -> size_t {
		return y * vwidth + x * 6;
	};

	for (int x = 0; x < size.x; x++) {

		std::array<Vec2f, 6> texpos;
		std::array<Color, 6> col;
		for (int i = 0; i < 6; i++) {
			texpos[i] = verts[at(x, size.y - 1) + i].tex_pos;
			col[i] = verts[at(x, size.y - 1) + i].color;
		}
		for (int y = size.y - 1; y >= 1; y--) {
			for (int i = 0; i < 6; i++) {
				verts[at(x, y) + i].tex_pos = verts[at(x, (size_t)y - 1) + i].tex_pos;
				verts[at(x, y) + i].color = verts[at(x, (size_t)y - 1) + i].color;
			}
		}
		for (int i = 0; i < 6; i++) {
			verts[at(x, 0) + i].tex_pos = texpos[i];
			verts[at(x, 0) + i].color = col[i];
		}
	}
}
void TileVertexArray::rotate_backwardY() {
	if (size.y <= 1)
		return;

	size_t vwidth = (size_t)size.x * 6;
	size_t vheight = (size_t)size.y * 6;

	auto at = [&vwidth](const size_t& x, const size_t& y) -> size_t {
		return y * vwidth + x * 6;
	};

	for (int x = 0; x < size.x; x++) {

		std::array<Vec2f, 6> texpos;
		std::array<Color, 6> col;

		for (int i = 0; i < 6; i++) {
			texpos[i] = verts[at(x, 0) + i].tex_pos;
			col[i] = verts[at(x, 0) + i].color;
		}
		for (int y = 0; y < size.y - 1; y++) {
			for (int i = 0; i < 6; i++) {
				verts[at(x, y) + i].tex_pos = verts[at(x, y + 1) + i].tex_pos;
				verts[at(x, y) + i].color = verts[at(x, y + 1) + i].color;
			}
		}
		for (int i = 0; i < 6; i++) {
			verts[at(x, size.y - 1) + i].tex_pos = texpos[i];
			verts[at(x, size.y - 1) + i].color = col[i];
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
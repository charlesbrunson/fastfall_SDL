#include "fastfall/render/drawable/TileArray.hpp"

#include "GL/glew.h"
#include "../detail/error.hpp"
#include "fastfall/render/render.hpp"

namespace ff {

//constexpr uint8_t MAX_DIMEN = 16u;

TileArray::TileArray(Vec2u size)
	: m_size(size)
{
	tile_count  = 0;
	tiles = grid_vector<TileID>(size.x, size.y);
	//std::fill(tiles.begin(), tiles.end(), TileID{});
}

TileArray::~TileArray()
{
	glStaleVertexArrays(gl.m_array);
	glStaleVertexBuffers(gl.m_buffer);
}

TileArray::TileArray(const TileArray& rhs)
	: m_size(rhs.m_size)
{
	offset = rhs.offset;
	m_tex = rhs.m_tex;
	tile_count = rhs.tile_count;
	tiles = rhs.tiles;

	gl.sync = false;
}

TileArray& TileArray::operator= (const TileArray& rhs)
{
	m_size = rhs.m_size;
	offset = rhs.offset;
	m_tex = rhs.m_tex;
	tile_count = rhs.tile_count;
	tiles = rhs.tiles;

	gl.sync = false;

	return *this;
}

TileArray::TileArray(TileArray&& rhs) noexcept
{
	offset = rhs.offset;
	m_size = rhs.m_size;
	m_tex = rhs.m_tex;
	tile_count = rhs.tile_count;

	std::swap(tiles, rhs.tiles);
	std::swap(gl, rhs.gl);
}
TileArray& TileArray::operator=(TileArray&& rhs) noexcept
{
	offset = rhs.offset;
	m_size = rhs.m_size;
	m_tex = rhs.m_tex;
	tile_count = rhs.tile_count;

	std::swap(tiles, rhs.tiles);
	std::swap(gl, rhs.gl);
	return *this;
}

void TileArray::setTexture(const Texture& texture) noexcept
{
	m_tex = texture;
}

const TextureRef& TileArray::getTexture() const noexcept
{
	return m_tex;
}

void TileArray::resize(Vec2u n_size)
{
	LOG_WARN("TileArray::resize is currently unsupported");
}

void TileArray::setTile(Vec2u at, TileID tile)
{
	assert(at.x < m_size.x && at.y < m_size.y);
	size_t ndx = ((size_t)at.y * m_size.x) + at.x;

	if (!tiles[ndx].valid() && tile.valid())
	{
		tile_count++;
	}
    if (tiles[ndx] != tile) {
        gl.sync = false;
    }
	tiles[ndx] = tile;
}

void TileArray::blank(Vec2u at)
{
	assert(at.x < m_size.x && at.y < m_size.y);
	size_t ndx = ((size_t)at.y * m_size.x) + at.x;

	if (tiles[ndx].valid()) 
	{
		tile_count--;
		gl.sync = false;
	}
	tiles[ndx] = {};
}

void TileArray::clear()
{
	m_tex = TextureRef{};
	tile_count = 0;

	std::fill(tiles.begin(), tiles.end(), TileID{});
	gl.sync = false;
}

void TileArray::glTransfer() const
{
	if (gl.m_array == 0) {
		// do the opengl initializaion
		glCheck(glGenVertexArrays(1, &gl.m_array));
		glCheck(glGenBuffers(1, &gl.m_buffer));

		glCheck(glBindVertexArray(gl.m_array));
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, gl.m_buffer));
		glCheck(glBufferData(GL_ARRAY_BUFFER, tiles.size() * sizeof(TileID), nullptr, GL_DYNAMIC_DRAW));
		gl.m_bufsize = tiles.size();
		
		// tile id attribute
		glCheck(glVertexAttribIPointer(0, 1, GL_UNSIGNED_SHORT, 0, (void*)0));
		glCheck(glEnableVertexAttribArray(0));
		glCheck(glVertexAttribDivisor(0, 1)); // one tile id per instance

		if (gl.m_array == 0 || gl.m_buffer == 0) {
			LOG_ERR_("Unable to initialize vertex array for opengl");
			assert(false);
		}
	}
	if (!gl.sync) {
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, gl.m_buffer));
		if (!gl.m_bound || tiles.size() > gl.m_bufsize) {
			glCheck(glBufferData(GL_ARRAY_BUFFER, tiles.size() * sizeof(TileID), tiles.data(), GL_DYNAMIC_DRAW));
			gl.m_bufsize = tiles.size();
			gl.m_bound = true;
		}
		else {
			glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, tiles.size() * sizeof(TileID), tiles.data()));
		}
		gl.sync = true;
	}
}

}

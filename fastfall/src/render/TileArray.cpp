#include "fastfall/render/TileArray.hpp"

#include "GL/glew.h"
#include "detail/error.hpp"

namespace ff {

constexpr uint8_t MAX_DIMEN = 16u;

TileArray::TileArray(Vec2u size)
	: m_size(size)
	, max_tiles(size.x * size.y)
{
	has_tile 	= std::make_unique<   bool[]>(max_tiles);
	tiles 		= std::make_unique<uint8_t[]>(max_tiles);

	std::fill_n(&has_tile[0], max_tiles, false);
	std::fill_n(&tiles[0], 	  max_tiles, 0);
}

void TileArray::setTexture(const Texture& texture) noexcept
{
	m_tex = texture;
}

const TextureRef& TileArray::getTexture() const noexcept
{
	return m_tex;
}

void TileArray::setTile(Vec2u at, Vec2u texPos)
{
	assert(at.x < m_size.x && at.y < m_size.y);
	size_t ndx = at.x + (at.y * m_size.x);

	assert(texPos.x < MAX_DIMEN && texPos.y < MAX_DIMEN);
	uint8_t tile_id = texPos.x + (texPos.y * MAX_DIMEN);

	if (!has_tile[ndx]) {
		has_tile[ndx] = true;
		tile_count++;
	}
	tiles[ndx] = tile_id;	
}

void TileArray::blank(Vec2u at)
{
	assert(at.x < m_size.x && at.y < m_size.y);
	size_t ndx = at.x + (at.y * m_size.x);

	if (has_tile[ndx]) {
		has_tile[ndx] = false;
		tile_count--;
	}
	tiles[ndx] = 0;	
}

void TileArray::clear()
{
	m_tex = TextureRef{};
	tile_count = 0;

	std::fill_n(&has_tile[0], max_tiles, false);
	std::fill_n(&tiles[0], 	  max_tiles, 0);
}

void TileArray::glTransfer() const
{
	if (gl.m_array == 0) {
		// do the opengl initializaion
		glCheck(glGenVertexArrays(1, &gl.m_array));
		glCheck(glGenBuffers(1, &gl.m_buffer));

		glCheck(glBindVertexArray(gl.m_array));
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, gl.m_buffer));
		glCheck(glBufferData(GL_ARRAY_BUFFER, max_tiles * sizeof(uint8_t), NULL, GL_DYNAMIC_DRAW));
		gl.m_bufsize = max_tiles;

		// tile id attribute
		glCheck(glEnableVertexAttribArray(0));
		glCheck(glVertexAttribPointer(0, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(uint8_t), (void*)0));

		if (gl.m_array == 0 || gl.m_buffer == 0) {
			LOG_ERR_("Unable to initialize vertex array for opengl");
			assert(false);
		}
	}
	if (!gl.sync) {
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, gl.m_buffer));
		glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0 * sizeof(uint8_t), max_tiles * sizeof(uint8_t), &tiles[0]));
		gl.sync = true;
	}
}

void TileArray::draw(RenderTarget& target, RenderState state) const
{
	target.draw(*this, state);
}

}

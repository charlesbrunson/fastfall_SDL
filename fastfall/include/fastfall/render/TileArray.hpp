#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/render/Transformable.hpp"
#include "fastfall/render.hpp"

namespace ff {

class TileArray : public Transformable {

public:
	TileArray(Vec2u size);
	~TileArray();

	TileArray(const TileArray& rhs);
	TileArray& operator= (const TileArray& rhs);

	TileArray(TileArray&& rhs);
	TileArray& operator=(TileArray&& rhs);

	void setTexture(const Texture& texture) noexcept;
	const TextureRef& getTexture() const noexcept;
	void setTile(Vec2u at, Vec2u texPos);
	void blank(Vec2u at);
	void clear();

	void resize(Vec2u n_size);

	constexpr inline bool empty() noexcept { 
		return tile_count == 0; 
	};

	Vec2f offset;

private:
	struct GPUState {
		GLuint m_array = 0;
		GLuint m_buffer = 0;

		size_t m_bufsize = 0;
		bool m_bound = false;

		bool sync = false;
	} mutable gl;

	friend class RenderTarget;
	void glTransfer() const;

private:
	TextureRef m_tex;

	Vec2u m_size;

	size_t tile_count;
	size_t max_tiles;
	std::unique_ptr<uint8_t[]> tiles;

};

}

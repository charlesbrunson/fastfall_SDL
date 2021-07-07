#pragma once

#include "TileVertexArray.hpp"

#include <vector>

namespace ff {


class ChunkVertexArray : public Drawable {
public:
	ChunkVertexArray(Vec2u t_size, Vec2u t_max_chunk_size);
	ChunkVertexArray(const ChunkVertexArray& rhs);
	ChunkVertexArray(ChunkVertexArray&& rhs);

	ChunkVertexArray& operator= (const ChunkVertexArray& rhs);
	ChunkVertexArray& operator= (ChunkVertexArray&& rhs);



	void setTexture(const Texture& texture) noexcept;
	const TextureRef& getTexture() const noexcept;
	void setTile(Vec2u at, Vec2u texPos);

	void blank(Vec2u at);
	void clear();

	inline bool empty() noexcept { return m_chunks.empty(); };

	void add_scroll(Vec2f scroll_amount);
	void reset_scroll();

	void predraw();

	Vec2f offset;

	bool use_visible_rect = false;
	Rectf visibility;

private:
	struct Chunk {
		Vec2u chunk_pos;
		Vec2u chunk_size;
		TileVertexArray tva;
	};

	Vec2f scroll;

	Vec2u m_size;
	Vec2u m_chunk_size;

	TextureRef m_tex;
	std::vector<Chunk> m_chunks;

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;
};

}
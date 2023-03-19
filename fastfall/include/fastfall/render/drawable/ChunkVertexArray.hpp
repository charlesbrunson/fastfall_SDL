#pragma once

#include "TileArray.hpp"
#include "TileVertexArray.hpp"

#include <vector>
#include <queue>

namespace ff {


class ChunkVertexArray : public Drawable {
public:
	using Array = TileArray;

	ChunkVertexArray(Vec2u t_size, Vec2u t_max_chunk_size);

	ChunkVertexArray(const ChunkVertexArray&) = default;
	ChunkVertexArray& operator=(const ChunkVertexArray&) = default;
	ChunkVertexArray(ChunkVertexArray&&) noexcept = default;
	ChunkVertexArray& operator=(ChunkVertexArray&&) noexcept = default;

	void setTexture(const Texture& texture) noexcept;
	const TextureRef& getTexture() const noexcept;

	void setTile(Vec2u at, TileID tile) { 
		commands.push(Command{ .type = Command::Type::Set, .tile_pos = at, .tile = tile });
	};
	void blank(Vec2u at) {
		commands.push(Command{ .type = Command::Type::Blank, .tile_pos = at });
	};
	void clear() {
		commands.push(Command{ .type = Command::Type::Clear });
	};

	void set_size(Vec2u size);

	inline bool empty() noexcept { return m_chunks.empty(); };

	void predraw(float interp, bool updated) override;

	Vec2f offset;
	Vec2f scroll;

	bool use_visible_rect = false;
	Rectf visibility;

private:
	void do_setTile(Vec2u at, TileID tile);
	void do_blank(Vec2u at);
	void do_clear();


	enum DrawFlags : unsigned char {
		NoDraw = 0,
		Draw = 1 << 0,
		DrawOffsetY = 1 << 1,
		DrawOffsetX = 1 << 2,
		DrawOffsetXY = 1 << 3
	};

	struct Chunk {
		unsigned char draw_flags;
		Vec2u chunk_pos;
		Vec2u chunk_size;
		Array tva;
	};

	Rectf getChunkBounds(const Chunk& chunk, Vec2f draw_offset = Vec2f{}) const noexcept;
	Rectf getChunkLocalBounds(const Chunk& chunk) const noexcept;

	Vec2u m_size;
	Vec2u m_chunk_size;

	TextureRef m_tex;
	std::vector<Chunk> m_chunks;

	struct Command {
		enum class Type {
			Set,
			Blank,
			Clear
		} type;

		Vec2u tile_pos;
		TileID tile;
	};
	std::queue<Command> commands;

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;
};

}

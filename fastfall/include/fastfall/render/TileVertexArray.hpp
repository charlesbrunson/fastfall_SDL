#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/render/VertexArray.hpp"
#include "fastfall/render/Drawable.hpp"

//#include <SFML/Graphics.hpp>

namespace ff {

class TileVertexArray : public Drawable {
public:
	TileVertexArray();
	TileVertexArray(Vec2u arr_size);

	~TileVertexArray();

	void setTexture(const Texture& texture) noexcept;
	const TextureRef& getTexture() noexcept;
	void setTile(Vec2u at, Vec2u texPos);

	void erase(Vec2u at);
	void blank(Vec2u at);
	void clear();

	void rotate_forwardX();
	void rotate_backwardX();
	void rotate_forwardY();
	void rotate_backwardY();

	inline bool empty() noexcept { return tiles.empty(); };

	Vec2f offset;

protected:


	TextureRef tex;
	Vec2u size;
	Vec2u rotation_offset{ 0, 0 };

	struct Tile {
		Tile() {};
		Tile(Vec2u at, Vec2u texPos) :
			position(at), tex_position(texPos)
		{};
		Vec2u position;
		Vec2u tex_position;
	};

	std::vector<Tile>    tiles;
	//std::vector<Vertex>  verts;
	VertexArray verts;

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;

};

}
#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/render/VertexArray.hpp"
#include "fastfall/render/Drawable.hpp"

namespace ff {

class TileVertexArray : public Drawable {
public:
	TileVertexArray();
	TileVertexArray(Vec2u arr_size);

	void setTexture(const Texture& texture) noexcept;
	const TextureRef& getTexture() const noexcept;
	void setTile(Vec2u at, Vec2u texPos);

	void resize(Vec2u size);

	void blank(Vec2u at);

	void clear();

	inline bool empty() noexcept { return m_verts.empty(); };

	Vec2f offset;

protected:

	TextureRef m_tex;
	Vec2u m_size;
	VertexArray m_verts;

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;

};

}
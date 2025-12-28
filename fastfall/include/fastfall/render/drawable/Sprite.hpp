#pragma once

#include "fastfall/render/drawable/VertexArray.hpp"
#include "fastfall/render/util/Transformable.hpp"
#include "fastfall/render/util/Texture.hpp"
#include "fastfall/util/Rect.hpp"
#include "Drawable.hpp"

namespace ff {

class Sprite : public Drawable, public Transformable {
public:
	Sprite();
	Sprite(const Texture* texture);
	Sprite(const Texture* texture, Vec2f spriteSize);
	Sprite(const Texture* texture, float spriteSizeX, float spriteSizeY);
	Sprite(const Texture* texture, Rectf textureRect, Vec2f spriteSize);
	Sprite(const Texture* texture, Rectf textureRect, float spriteSizeX, float spriteSizeY);

	void setTexture(const Texture* texture, bool resetRect = false);
	void setTextureRect(Rectf textureRect);
	void setColor(Color color);
	void setSize(Vec2f size);
	void setSize(float sizeX, float sizeY);

	Rectf getTextureRect() const;
	const Texture* getTexture() const;
	Color getColor() const;
	Vec2f getSize() const;

private:
	void init();

	void draw(RenderTarget& target, RenderState state) const override;

	Color m_color;
	TextureRef m_texture;
	Rectf m_textureRect;
	Vec2f m_size;

	VertexArray m_verts;
};

}
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
	Sprite(const Texture* texture, glm::fvec2 spriteSize);
	Sprite(const Texture* texture, float spriteSizeX, float spriteSizeY);
	Sprite(const Texture* texture, Rectf textureRect, glm::fvec2 spriteSize);
	Sprite(const Texture* texture, Rectf textureRect, float spriteSizeX, float spriteSizeY);

	void setTexture(const Texture* texture, bool resetRect = false);
	void setTextureRect(Rectf textureRect);
	void setColor(Color color);
	void setSize(glm::fvec2 size);
	void setSize(float sizeX, float sizeY);

	Rectf getTextureRect() const;
	const Texture* getTexture() const;
	Color getColor() const;
	glm::fvec2 getSize() const;

private:
	void init();

	void draw(RenderTarget& target, RenderState state) const override;

	Color m_color;
	TextureRef m_texture;
	Rectf m_textureRect;
	glm::fvec2 m_size;

	VertexArray m_verts;
};

}
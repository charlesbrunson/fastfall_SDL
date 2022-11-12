#include "fastfall/render/Sprite.hpp"
#include "fastfall/render/RenderTarget.hpp"

namespace ff {


	Sprite::Sprite()
	: m_verts{ Primitive::TRIANGLE_STRIP, 4 },
	m_color{ ff::Color::White }
{

	setTexture(nullptr);
	init();
}

Sprite::Sprite(const Texture* texture)
	: m_verts{ Primitive::TRIANGLE_STRIP, 4 },
	m_color{ ff::Color::White }
{

	setTexture(texture, true);
	init();
}


Sprite::Sprite(const Texture* texture, glm::fvec2 spriteSize)
	: m_verts{ Primitive::TRIANGLE_STRIP, 4 },
	m_color{ ff::Color::White },
	m_size{ spriteSize }
{
	setTexture(texture, true);
	if (m_texture.exists()) {
		m_textureRect = Rectf{
			{0.f, 0.f},
			glm::fvec2{m_texture.get()->size()}
		};
	}
	init();
}
	
Sprite::Sprite(const Texture* texture, float spriteSizeX, float spriteSizeY)
	:Sprite{ texture, {spriteSizeX, spriteSizeY} }
{

}

Sprite::Sprite(const Texture* texture, Rectf textureRect, glm::fvec2 spriteSize)
	: m_verts{Primitive::TRIANGLE_FAN, 4},
	m_color{ff::Color::White},
	m_textureRect{textureRect},
	m_size{spriteSize}
{
	setTexture(texture);
	init();
}


Sprite::Sprite(const Texture* texture, Rectf textureRect, float spriteSizeX, float spriteSizeY)
	: Sprite(texture, textureRect, { spriteSizeX, spriteSizeY })
{

}

void Sprite::setTexture(const Texture* texture, bool resetRect) {
	m_texture = texture ? *texture : Texture::getNullTexture();

	if (resetRect && m_texture.exists()) {
		setTextureRect(Rectf{
				{ 0, 0 },
				m_texture.get()->size()
			});
		m_size = m_texture.get()->size();
	}
}

void Sprite::setTextureRect(Rectf textureRect) {

	if (m_textureRect != textureRect) {
		m_textureRect = textureRect;

        constexpr float tex_offset = 1.f / 16384.f;
        auto inv_size = m_texture.get()->inverseSize();
		auto points = m_textureRect.toPoints();
		m_verts[0].tex_pos = points[0] * inv_size;
		m_verts[1].tex_pos = points[1] * inv_size;
		m_verts[2].tex_pos = points[2] * inv_size;
		m_verts[3].tex_pos = points[3] * inv_size;

        m_verts[0].tex_pos += glm::vec2{  tex_offset,  tex_offset } * inv_size;
        m_verts[1].tex_pos += glm::vec2{ -tex_offset,  tex_offset } * inv_size;
        m_verts[2].tex_pos += glm::vec2{  tex_offset, -tex_offset } * inv_size;
        m_verts[3].tex_pos += glm::vec2{ -tex_offset, -tex_offset } * inv_size;
	}


	//m_verts.glTransfer();
}

void Sprite::setColor(Color color) {
	if (m_color != color) {
		m_color = color;
		m_verts[0].color = m_color;
		m_verts[1].color = m_color;
		m_verts[2].color = m_color;
		m_verts[3].color = m_color;
	}
	//m_verts.glTransfer();
}

void Sprite::setSize(glm::fvec2 size) {
	setSize(size.x, size.y);
}
void Sprite::setSize(float sizeX, float sizeY) {
	if (m_size.x != sizeX || m_size.y != sizeY) {
		m_size = { sizeX, sizeY };
		m_verts[0].pos = { 0.f, 0.f };
		m_verts[1].pos = { m_size.x, 0.f };
		m_verts[2].pos = { 0.f, m_size.y };
		m_verts[3].pos = { m_size.x, m_size.y };
	}
}

Rectf Sprite::getTextureRect() const {
	return m_textureRect;
}
const Texture* Sprite::getTexture() const {
	return m_texture.get();
}
Color Sprite::getColor() const {
	return m_color;
}

glm::fvec2 Sprite::getSize() const {
	return m_size;
}

void Sprite::init() {
	if (m_texture.exists()) {

		m_verts[0].pos = { 0.f, 0.f };
		m_verts[1].pos = { m_size.x, 0.f };
		m_verts[2].pos = { 0.f, m_size.y };
		m_verts[3].pos = { m_size.x, m_size.y };

		auto points = m_textureRect.toPoints();
		m_verts[0].tex_pos = points[0] * m_texture.get()->inverseSize();
		m_verts[1].tex_pos = points[1] * m_texture.get()->inverseSize();
		m_verts[2].tex_pos = points[2] * m_texture.get()->inverseSize();
		m_verts[3].tex_pos = points[3] * m_texture.get()->inverseSize();

		m_verts[0].color = m_color;
		m_verts[1].color = m_color;
		m_verts[2].color = m_color;
		m_verts[3].color = m_color;
		//m_verts.glTransfer();
	}
}

void Sprite::draw(RenderTarget& target, RenderState state) const {
	state.texture = m_texture;
	state.transform = Transform::combine(state.transform, getTransform());
	target.draw(m_verts, state);
}

}
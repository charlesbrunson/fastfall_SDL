#include "fastfall/render/drawable/ShapeRectangle.hpp"
#include "fastfall/render/target/RenderTarget.hpp"

namespace ff {


ShapeRectangle::ShapeRectangle()
	: m_verts{ Primitive::TRIANGLE_STRIP, 4 },
	m_outline_verts{ Primitive::LINE_LOOP, 4 },
	m_color{ Color::Transparent },
	m_lineColor{ Color::Transparent },
	m_area{ 0.f, 0.f, 0.f, 0.f }
{

}

ShapeRectangle::ShapeRectangle(Rectf area, Color color, Color lineColor)
	: m_verts{ Primitive::TRIANGLE_STRIP, 4 },
	m_outline_verts{ Primitive::LINE_LOOP, 4 },
	m_color{color},
	m_lineColor{lineColor},
	m_area{area}
{

	setPosition(area.left, area.top);

	initVertexArray();
}

void ShapeRectangle::setSize(Vec2f size) {
	if (size != m_area.getSize()) {
		m_area.setSize(size);

		m_verts[0].pos = Vec2f{ 0.f, 0.f };
		m_verts[1].pos = Vec2f{ m_area.width, 0.f };
		m_verts[2].pos = Vec2f{ 0.f, m_area.height };
		m_verts[3].pos = Vec2f{ m_area.getSize() };

		m_outline_verts[0].pos = m_verts[0].pos;
		m_outline_verts[1].pos = m_verts[1].pos;
		m_outline_verts[2].pos = m_verts[3].pos;
		m_outline_verts[3].pos = m_verts[2].pos;
	}
}

void ShapeRectangle::setSize(float sizeX, float sizeY) {
	setSize(Vec2f{ sizeX, sizeY });
}

Vec2f ShapeRectangle::getSize() const {
	return m_area.getSize();
}

void ShapeRectangle::initVertexArray() {
	m_verts[0].pos = Vec2f{ 0.f, 0.f };
	m_verts[1].pos = Vec2f{ m_area.width, 0.f };
	m_verts[2].pos = Vec2f{ 0.f, m_area.height };
	m_verts[3].pos = Vec2f{ m_area.getSize() };

	m_verts[0].color = m_color;
	m_verts[1].color = m_color;
	m_verts[2].color = m_color;
	m_verts[3].color = m_color;

	m_outline_verts[0].pos = m_verts[0].pos;
	m_outline_verts[1].pos = m_verts[1].pos;
	m_outline_verts[2].pos = m_verts[3].pos;
	m_outline_verts[3].pos = m_verts[2].pos;

	m_outline_verts[0].color = m_lineColor;
	m_outline_verts[1].color = m_lineColor;
	m_outline_verts[2].color = m_lineColor;
	m_outline_verts[3].color = m_lineColor;

	//m_verts.glTransfer();
	//m_outline_verts.glTransfer();
}

void ShapeRectangle::setColor(Color color) {
	m_color = color;
	m_verts[0].color = color;
	m_verts[1].color = color;
	m_verts[2].color = color;
	m_verts[3].color = color;
	//m_verts.glTransfer();
}

Color ShapeRectangle::getColor() const {
	return m_color;
}

void ShapeRectangle::setOutlineColor(Color color) {
	m_lineColor = color;
	m_outline_verts[0].color = color;
	m_outline_verts[1].color = color;
	m_outline_verts[2].color = color;
	m_outline_verts[3].color = color;
	//m_outline_verts.glTransfer();
}

Color ShapeRectangle::getOutlineColor() const {
	return m_lineColor;
}

void ShapeRectangle::draw(RenderTarget& target, RenderState state) const {

	state.transform = Transform::combine(state.transform, getTransform());
	if (m_color.a > 0) {
		target.draw(m_verts, state);
	}
	if (m_lineColor.a > 0) {
		target.draw(m_outline_verts, state);
	}
}

}
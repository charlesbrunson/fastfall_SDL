#include "fastfall/render/ShapeRectangle.hpp"
#include "fastfall/render/RenderTarget.hpp"

namespace ff {

ShapeRectangle::ShapeRectangle(Rectf area, Color color, Color lineColor)
	: m_verts{ ff::Primitive::TRIANGLE_STRIP, 4 },
	m_outline_verts{ ff::Primitive::LINE_LOOP, 4 },
	m_color{color},
	m_lineColor{lineColor},
	m_area{area}
{

	setPosition(area.left, area.top);

	initVertexArray();
}

void ShapeRectangle::setSize(glm::fvec2 size) {
	//m_size = size;
	m_area.setSize(size);

	m_verts[0].pos = glm::fvec2{ 0.f, 0.f };
	m_verts[1].pos = glm::fvec2{ m_area.width, 0.f };
	m_verts[2].pos = glm::fvec2{ 0.f, m_area.height };
	m_verts[3].pos = glm::fvec2{ m_area.getSize() };

	m_outline_verts[0].pos = m_verts[0].pos;
	m_outline_verts[1].pos = m_verts[1].pos;
	m_outline_verts[2].pos = m_verts[3].pos;
	m_outline_verts[3].pos = m_verts[2].pos;

	m_verts.glTransfer();
	m_outline_verts.glTransfer();
}

void ShapeRectangle::setSize(float sizeX, float sizeY) {
	setSize(glm::fvec2{ sizeX, sizeY });
}

glm::fvec2 ShapeRectangle::getSize() {
	return m_area.getSize();
}

void ShapeRectangle::initVertexArray() {
	m_verts[0].pos = glm::fvec2{ 0.f, 0.f };
	m_verts[1].pos = glm::fvec2{ m_area.width, 0.f };
	m_verts[2].pos = glm::fvec2{ 0.f, m_area.height };
	m_verts[3].pos = glm::fvec2{ m_area.getSize() };

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

	m_verts.glTransfer();
	m_outline_verts.glTransfer();
}

void ShapeRectangle::setColor(ff::Color color) {
	m_color = color;
	m_verts[0].color = color;
	m_verts[1].color = color;
	m_verts[2].color = color;
	m_verts[3].color = color;
	m_verts.glTransfer();
}

ff::Color ShapeRectangle::getColor() {
	return m_color;
}

void ShapeRectangle::setOutlineColor(ff::Color color) {
	m_lineColor = color;
	m_outline_verts[0].color = color;
	m_outline_verts[1].color = color;
	m_outline_verts[2].color = color;
	m_outline_verts[3].color = color;
	m_outline_verts.glTransfer();
}

ff::Color ShapeRectangle::getOutlineColor() {
	return m_lineColor;
}

void ShapeRectangle::draw(RenderTarget& target, RenderState state) const {

	state.transform = Transform::combine(state.transform, getTransform());
	target.draw(m_verts, state);
	target.draw(m_outline_verts, state);
}

}
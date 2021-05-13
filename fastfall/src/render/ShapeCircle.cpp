#include "fastfall/render/ShapeCircle.hpp"
#include "fastfall/render/RenderTarget.hpp"

namespace ff {

ShapeCircle::ShapeCircle(glm::fvec2 center, float radius, size_t vertexCount, Color color, Color lineColor)
	: m_verts{ ff::Primitive::TRIANGLE_FAN, vertexCount + 2 },
	m_outline_verts{ ff::Primitive::LINE_LOOP, vertexCount },
	m_radius{ radius },
	m_color{color},
	m_lineColor{lineColor},
	m_circleVertCount{ vertexCount }
{
	initVertexArray();
	setPosition(center);
}

void ShapeCircle::setColor(ff::Color color) {
	m_color = color;

	m_verts[0].color = m_color;
	for (size_t step = 1; step < m_circleVertCount + 1; step++) {
		m_verts[step].color = m_color;
	}
	m_verts[m_circleVertCount + 1].color = m_color;

	m_verts.glTransfer();
}

void ShapeCircle::setOutlineColor(ff::Color color) {
	m_lineColor = color;
	for (size_t step = 0; step < m_circleVertCount; step++) {
		m_outline_verts[step].color = m_lineColor;
	}
	m_outline_verts.glTransfer();
}

void ShapeCircle::setRadius(float radius) {
	m_radius = radius;
	initVertexArray();
}

void ShapeCircle::initVertexArray() {

	// init circle
	m_verts[0].pos = glm::fvec2{ 0.f, 0.f };
	m_verts[0].color = m_color;

	float angle = (M_PI * 2.f) / (float)m_circleVertCount;
	for (size_t step = 1; step < m_circleVertCount + 1; step++) {
		m_verts[step].pos = glm::fvec2(
			cosf((step - 1) * angle) * m_radius,
			sinf((step - 1) * angle) * m_radius
		);
		m_verts[step].color = m_color;

		m_outline_verts[step - 1].pos = glm::fvec2(
			cosf((step - 1) * angle) * m_radius,
			sinf((step - 1) * angle) * m_radius
		);
		m_outline_verts[step - 1].color = m_lineColor;
	}

	m_verts[m_circleVertCount + 1].pos = glm::fvec2(
		cosf(0) * m_radius,
		sinf(0) * m_radius
	);
	m_verts[m_circleVertCount + 1].color = m_color;
	m_verts.glTransfer();
	m_outline_verts.glTransfer();
}

void ShapeCircle::draw(RenderTarget& target, RenderState state) const {
	state.transform = Transform::combine(state.transform, getTransform());
	target.draw(m_verts, state);
	target.draw(m_outline_verts, state);
}

}
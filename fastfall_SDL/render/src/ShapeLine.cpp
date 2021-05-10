#include "render/ShapeLine.hpp"
#include "render/RenderTarget.hpp"

namespace ff {

void ShapeLine::setP1(glm::fvec2 point) {
	m_verts[0].pos = point;
	m_verts.glTransfer(0, 1);
}

void ShapeLine::setP2(glm::fvec2 point) {
	m_verts[0].pos = point;
	m_verts.glTransfer(1, 1);
}

void ShapeLine::setPoint(unsigned ndx, glm::fvec2 point) {
	m_verts[ndx].pos = point;
	m_verts.glTransfer(ndx, 1);
}

void ShapeLine::set(glm::fvec2 p1, glm::fvec2 p2) {
	m_verts[0].pos = p1;
	m_verts[1].pos = p2;
	m_verts.glTransfer();
}

void ShapeLine::setColor(Color color) {
	m_verts[0].color = color;
	m_verts[1].color = color;
	m_verts.glTransfer();
}

void ShapeLine::draw(RenderTarget& target, RenderState state) const {
	state.transform = Transform::combine(state.transform, getTransform());
	target.draw(m_verts, state);
}

}
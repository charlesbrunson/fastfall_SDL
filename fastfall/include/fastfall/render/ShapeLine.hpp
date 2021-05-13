#pragma once

#include "VertexArray.hpp"

namespace ff {

class RenderTarget;

class ShapeLine : public Transformable, public Drawable {
public:
	ShapeLine(glm::fvec2 p1, glm::fvec2 p2, Color color = ff::Color::White) 
		: m_verts{ ff::Primitive::LINES, 2 }
	{

		m_verts[0].pos = p1;
		m_verts[1].pos = p2;
		m_verts[0].color = color;
		m_verts[1].color = color;
		m_verts.glTransfer();
	}

	inline glm::fvec2 getP1() { return m_verts[0].pos; }
	inline glm::fvec2 getP2() { return m_verts[1].pos; }
	inline glm::fvec2 getPoint(unsigned ndx) { return m_verts[ndx].pos; }

	void setP1(glm::fvec2 point);
	void setP2(glm::fvec2 point);
	void setPoint(unsigned ndx, glm::fvec2 point);
	void set(glm::fvec2 p1, glm::fvec2 p2);

	void setColor(Color color);

private:
	VertexArray m_verts;

	void draw(RenderTarget& target, RenderState state) const override;
};

}
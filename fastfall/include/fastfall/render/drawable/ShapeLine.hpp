#pragma once

#include "fastfall/render/drawable/VertexArray.hpp"
#include "Drawable.hpp"

namespace ff {

class RenderTarget;

class ShapeLine : public Transformable, public Drawable {
public:
	ShapeLine(Vec2f p1, Vec2f p2, Color color = Color::White)
		: m_verts{ Primitive::LINES, 2 }
	{

		m_verts[0].pos = p1;
		m_verts[1].pos = p2;
		m_verts[0].color = color;
		m_verts[1].color = color;
	}

	Vec2f getP1() { return m_verts[0].pos; }
	Vec2f getP2() { return m_verts[1].pos; }
	Vec2f getPoint(unsigned ndx) { return m_verts[ndx].pos; }

	void setP1(Vec2f point);
	void setP2(Vec2f point);
	void setPoint(unsigned ndx, Vec2f point);
	void set(Vec2f p1, Vec2f p2);

	void setColor(Color color);

private:
	VertexArray m_verts;

	void draw(RenderTarget& target, RenderState state) const override;
};

}
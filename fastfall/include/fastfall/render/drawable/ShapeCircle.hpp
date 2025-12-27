#pragma once

#include "fastfall/render/drawable/VertexArray.hpp"
#include "Drawable.hpp"

namespace ff {

class RenderTarget;

class ShapeCircle : public Transformable, public Drawable {
public:
	ShapeCircle(glm::fvec2 center, float radius, size_t vertexCount = 8, Color color = ff::Color::White, Color lineColor = ff::Color::Transparent);

	void setColor(ff::Color color);
	void setOutlineColor(ff::Color color);
	void setRadius(float radius);

private:
	void initVertexArray();

	Color m_color;
	Color m_lineColor;

	float m_radius;

	VertexArray m_verts;
	VertexArray m_outline_verts;

	size_t m_circleVertCount;

	void draw(RenderTarget& target, RenderState state) const override;
};

}
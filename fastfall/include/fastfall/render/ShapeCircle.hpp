#pragma once

#include "VertexArray.hpp"
#include "fastfall/render/Drawable.hpp"

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

	ff::Color m_color;
	ff::Color m_lineColor;

	float m_radius;

	VertexArray m_verts;
	VertexArray m_outline_verts;

	size_t m_circleVertCount;

	void draw(RenderTarget& target, RenderState state) const override;
};

}
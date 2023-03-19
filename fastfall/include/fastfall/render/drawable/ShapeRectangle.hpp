#pragma once

#include "fastfall/render/drawable/VertexArray.hpp"
#include "fastfall/util/Rect.hpp"
#include "Drawable.hpp"

namespace ff {

class ShapeRectangle : public Transformable, public Drawable {
public:
	ShapeRectangle();
	ShapeRectangle(Rectf area, Color color = ff::Color::White, Color lineColor = ff::Color::Transparent);

	void setSize(glm::fvec2 size);
	void setSize(float sizeX, float sizeY);
	glm::fvec2 getSize() const;

	void setColor(ff::Color color);
	Color getColor() const;

	void setOutlineColor(ff::Color color);
	Color getOutlineColor() const;

protected:
	void initVertexArray();

	Color m_color;
	Color m_lineColor;

	Rectf m_area;

	VertexArray m_verts;
	VertexArray m_outline_verts;

	void draw(RenderTarget& target, RenderState state) const override;
};

}
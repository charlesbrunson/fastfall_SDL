#pragma once

#include "fastfall/render/drawable/VertexArray.hpp"
#include "fastfall/util/Rect.hpp"
#include "Drawable.hpp"

namespace ff {

class ShapeRectangle : public Transformable, public Drawable {
public:
	ShapeRectangle();
	ShapeRectangle(Rectf area, Color color = Color::White, Color lineColor = Color::Transparent);

	void setSize(Vec2f size);
	void setSize(float sizeX, float sizeY);
	Vec2f getSize() const;

	void setColor(Color color);
	Color getColor() const;

	void setOutlineColor(Color color);
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
#pragma once

#include "glm/glm.hpp"
#include "Transform.hpp"
#include "fastfall/util/glm_types.hpp"
#include "fastfall/util/Rect.hpp"

namespace ff {

class View {
public:
	View();
	View(Vec2f botleft, Vec2f size, Vec2f scale = {1.f, 1.f});

	void setCenter(Vec2f center);
	void setViewport(Rectf viewport);
	void setSize(Vec2f size);
	void setSize(float sizeX, float sizeY);
	void setZoom(float zoom);

	Vec2f getCenter() const;
	Rectf getViewport() const;
	Vec2f getSize() const;
	float getZoom() const;

	glm::mat3 getMatrix() const;
	glm::mat3 getInvMatrix() const;

	//void setScale(glm::fvec2 scale) { m_scale_mult = scale; };

private:
	Vec2f m_center;
	Rectf  m_viewport;
	Vec2f m_size;
	float m_zoom = 1.f;

	Vec2f m_scale_mult;
};

}

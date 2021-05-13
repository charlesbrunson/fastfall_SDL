#pragma once

#include <glm/glm.hpp>
#include "Transform.hpp"

namespace ff {

class View {
public:
	View();
	View(glm::fvec2 botleft, glm::fvec2 size, glm::fvec2 scale = {1.f, 1.f});

	void setCenter(glm::fvec2 center);
	void setViewport(glm::vec4 viewport);
	void setSize(glm::fvec2 size);
	void setSize(float sizeX, float sizeY);
	void setZoom(float zoom);

	glm::fvec2 getCenter() const;
	glm::vec4 getViewport() const;
	glm::fvec2 getSize() const;
	float getZoom() const;

	glm::mat3 getMatrix() const;
	glm::mat3 getInvMatrix() const;

	//void setScale(glm::fvec2 scale) { m_scale_mult = scale; };

private:
	glm::fvec2 m_center;
	glm::vec4  m_viewport;
	glm::fvec2 m_size;
	float m_zoom = 1.f;

	glm::fvec2 m_scale_mult;
};

}
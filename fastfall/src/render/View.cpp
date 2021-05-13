#include "fastfall/render/View.hpp"

#include <glm/gtx/matrix_transform_2d.hpp>

namespace ff {

View::View()
{
	setViewport(glm::vec4{ 0, 0, 640, 480 });
	setCenter({ 0.f, 0.f });
	setZoom(1.f);
	m_scale_mult = {1.f, 1.f};
}

View::View(glm::fvec2 botleft, glm::fvec2 size, glm::fvec2 scale)
{
	setViewport(glm::vec4{ botleft, size });
	setCenter({ 0.f, 0.f });
	setZoom(1.f);
	m_scale_mult = scale;
}

void View::setCenter(glm::fvec2 center) {
	m_center = center;
}

void View::setViewport(glm::vec4 viewport) {
	m_viewport = viewport;
	m_size = glm::fvec2{
		m_viewport[2],
		m_viewport[3]
	};
}

void View::setSize(glm::fvec2 size) {
	m_size = size;
}

void View::setSize(float sizeX, float sizeY) {
	m_size = glm::fvec2{ sizeX, sizeY };
}

void View::setZoom(float zoom) {
	m_zoom = zoom;
}


glm::fvec2 View::getCenter() const {
	return m_center; 
}

glm::vec4 View::getViewport() const { 
	return m_viewport; 
}

glm::fvec2 View::getSize() const {
	return m_size; 
}

float View::getZoom() const {
	return m_zoom;
}


glm::mat3 View::getMatrix() const {
	glm::mat3 mat;
	mat = glm::scale(glm::mat3(1.0f), glm::fvec2{ (m_zoom * 2.f) / m_size } * m_scale_mult);
	mat = glm::translate(mat, m_center * glm::fvec2(-1.f, 1.f));
	return  mat;
}
glm::mat3 View::getInvMatrix() const {
	glm::mat3 mat;
	mat = glm::translate(glm::mat3(1.0f), m_center * glm::fvec2(1.f, -1.f));
	mat = glm::scale(mat, glm::fvec2{ m_size / (m_zoom * 2.f) } * m_scale_mult);
	return  mat;
}


}
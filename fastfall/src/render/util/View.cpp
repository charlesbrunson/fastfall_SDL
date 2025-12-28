#include "fastfall/render/util/View.hpp"

#include "fastfall/util/glm_types.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"

namespace ff {

View::View()
{
	setViewport(Rectf{ 0, 0, 640, 480 });
	setCenter({ 0.f, 0.f });
	setZoom(1.f);
	m_scale_mult = {1.f, 1.f};
}

View::View(Vec2f botleft, Vec2f size, Vec2f scale)
{
	setViewport(Rectf{ botleft, size });
	setCenter({ 0.f, 0.f });
	setZoom(1.f);
	m_scale_mult = scale;
}

void View::setCenter(Vec2f center) {
	m_center = center;
}

void View::setViewport(Rectf viewport) {
	m_viewport = viewport;
	m_size = glm::fvec2{
		m_viewport.width,
		m_viewport.height
	};
}

void View::setSize(Vec2f size) {
	m_size = size;
}

void View::setSize(float sizeX, float sizeY) {
	m_size = Vec2f{ sizeX, sizeY };
}

void View::setZoom(float zoom) {
	m_zoom = zoom;
}


Vec2f View::getCenter() const {
	return m_center; 
}

Rectf View::getViewport() const {
	return m_viewport; 
}

Vec2f View::getSize() const {
	return m_size; 
}

float View::getZoom() const {
	return m_zoom;
}


glm::mat3 View::getMatrix() const {
	glm::mat3 mat;
	mat = glm::scale(glm::mat3(1.0f), Vec2f{ (m_zoom * 2.f) / m_size } * m_scale_mult);
	mat = glm::translate(mat, m_center * Vec2f(-1.f, 1.f));
	return  mat;
}
glm::mat3 View::getInvMatrix() const {
	glm::mat3 mat;
	mat = glm::translate(glm::mat3(1.0f), m_center * Vec2f(1.f, -1.f));
	mat = glm::scale(mat, Vec2f{ m_size / (m_zoom * 2.f) } * m_scale_mult);
	return  mat;
}


}
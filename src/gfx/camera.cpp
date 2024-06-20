#include "ff/gfx/camera.hpp"

#include "glm/glm.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_transform_2d.hpp"
#undef GLM_ENABLE_EXPERIMENTAL

namespace ff {

camera::camera()
{
	set_viewport({ 0, 0, 640, 480 });
	set_center({ 0.f, 0.f });
	set_zoom(1.f);
	m_scale = {1.f, 1.f};
}

camera::camera(glm::vec2 t_botleft, glm::vec2 t_size, glm::vec2 t_scale)
{
	set_viewport({ t_botleft, t_size });
	set_center({ 0.f, 0.f });
	set_zoom(1.f);
	m_scale = t_scale;
}

void camera::set_center(glm::vec2 t_center) {
	m_center = t_center;
}

void camera::set_viewport(rectf t_viewport) {
	m_viewport = t_viewport;
	m_size = glm::vec2{
		m_viewport.width,
		m_viewport.height
	};
}

void camera::set_size(glm::vec2 t_size) {
	m_size = t_size;
}

void camera::set_zoom(float t_zoom) {
	m_zoom = t_zoom;
}

glm::vec2 camera::get_center() const {
	return m_center;
}

rectf camera::get_viewport() const {
	return m_viewport;
}

glm::vec2 camera::get_size() const {
	return m_size;
}

float camera::get_zoom() const {
	return m_zoom;
}

glm::mat3 camera::matrix() const {
	glm::mat3 mat;
	mat = glm::scale(glm::mat3(1.0f), glm::fvec2{ (m_zoom * 2.f) / m_size } * m_scale);
	mat = glm::translate(mat, m_scale * glm::fvec2(-1.f, 1.f));
	return  mat;
}

glm::mat3 camera::inv_matrix() const {
	glm::mat3 mat;
	mat = glm::translate(glm::mat3(1.0f), m_center * glm::fvec2(1.f, -1.f));
	mat = glm::scale(mat, glm::fvec2{ m_size / (m_zoom * 2.f) } * m_scale);
	return  mat;
}


}
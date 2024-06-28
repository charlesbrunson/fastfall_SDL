#include "ff/gfx/view.hpp"

#include "glm/glm.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_transform_2d.hpp"
#undef GLM_ENABLE_EXPERIMENTAL

namespace ff {

view::view()
{
	set_viewport({ 0, 0, 640, 480 });
	set_center({ 0.f, 0.f });
	set_zoom(1.f);
	m_scale = {1.f, 1.f};
}

view::view(vec2 t_botleft, vec2 t_size, vec2 t_scale)
{
	set_viewport({ t_botleft, t_size });
	set_center({ 0.f, 0.f });
	set_zoom(1.f);
	m_scale = t_scale;
}

void view::set_center(vec2 t_center) {
	m_center = t_center;
}

void view::set_viewport(rectf t_viewport) {
	m_viewport = t_viewport;
	m_size = vec2{
		m_viewport.width,
		m_viewport.height
	};
}

void view::set_size(vec2 t_size) {
	m_size = t_size;
}

void view::set_zoom(float t_zoom) {
	m_zoom = t_zoom;
}

vec2 view::get_center() const {
	return m_center;
}

rectf view::get_viewport() const {
	return m_viewport;
}

vec2 view::get_size() const {
	return m_size;
}

float view::get_zoom() const {
	return m_zoom;
}

mat3 view::matrix() const {
	mat3 mat;
	mat = scale(mat3(1.0f), fvec2{ (m_zoom * 2.f) / m_size } * m_scale);
	mat = translate(mat, m_scale * fvec2(-1.f, 1.f));
	return  mat;
}

mat3 view::inv_matrix() const {
	mat3 mat;
	mat = translate(mat3(1.0f), m_center * fvec2(1.f, -1.f));
	mat = scale(mat, fvec2{ m_size / (m_zoom * 2.f) } * m_scale);
	return  mat;
}


}
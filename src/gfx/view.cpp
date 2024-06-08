#include "ff/gfx/view.hpp"

#include "glm/glm.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"

namespace ff {

view::view()
{
	set_viewport({ 0, 0, 640, 480 });
	set_center({ 0.f, 0.f });
	set_zoom(1.f);
	scale = {1.f, 1.f};
}

view::view(vec2f botleft, vec2f size, vec2f n_scale)
{
	set_viewport({ botleft, size });
	set_center({ 0.f, 0.f });
	set_zoom(1.f);
	scale = n_scale;
}

void view::set_center(vec2f n_center) {
	center = n_center;
}

void view::set_viewport(rectf n_viewport) {
	viewport = n_viewport;
	size = vec2f{
		viewport.width,
		viewport.height
	};
}

void view::set_size(vec2f n_size) {
	size = n_size;
}

void view::set_zoom(float n_zoom) {
	zoom = n_zoom;
}

vec2f view::get_center() const {
	return center;
}

rectf view::get_viewport() const {
	return viewport;
}

vec2f view::get_size() const {
	return size;
}

float view::get_zoom() const {
	return zoom;
}

glm::mat3 view::matrix() const {
	glm::mat3 mat;
	mat = glm::scale(glm::mat3(1.0f), glm::fvec2{ (zoom * 2.f) / size } * scale);
	mat = glm::translate(mat, scale * glm::fvec2(-1.f, 1.f));
	return  mat;
}

glm::mat3 view::inv_matrix() const {
	glm::mat3 mat;
	mat = glm::translate(glm::mat3(1.0f), center * glm::fvec2(1.f, -1.f));
	mat = glm::scale(mat, glm::fvec2{ size / (zoom * 2.f) } * scale);
	return  mat;
}


}
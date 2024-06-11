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
	scale = {1.f, 1.f};
}

camera::camera(glm::vec2 botleft, glm::vec2 size, glm::vec2 n_scale)
{
	set_viewport({ botleft, size });
	set_center({ 0.f, 0.f });
	set_zoom(1.f);
	scale = n_scale;
}

void camera::set_center(glm::vec2 n_center) {
	center = n_center;
}

void camera::set_viewport(rectf n_viewport) {
	viewport = n_viewport;
	size = glm::vec2{
		viewport.width,
		viewport.height
	};
}

void camera::set_size(glm::vec2 n_size) {
	size = n_size;
}

void camera::set_zoom(float n_zoom) {
	zoom = n_zoom;
}

glm::vec2 camera::get_center() const {
	return center;
}

rectf camera::get_viewport() const {
	return viewport;
}

glm::vec2 camera::get_size() const {
	return size;
}

float camera::get_zoom() const {
	return zoom;
}

glm::mat3 camera::matrix() const {
	glm::mat3 mat;
	mat = glm::scale(glm::mat3(1.0f), glm::fvec2{ (zoom * 2.f) / size } * scale);
	mat = glm::translate(mat, scale * glm::fvec2(-1.f, 1.f));
	return  mat;
}

glm::mat3 camera::inv_matrix() const {
	glm::mat3 mat;
	mat = glm::translate(glm::mat3(1.0f), center * glm::fvec2(1.f, -1.f));
	mat = glm::scale(mat, glm::fvec2{ size / (zoom * 2.f) } * scale);
	return  mat;
}


}
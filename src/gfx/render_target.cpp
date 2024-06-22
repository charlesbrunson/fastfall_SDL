#include "ff/gfx/render_target.hpp"

#include "../external/glew.hpp"

namespace ff {

render_target::render_target()
	: m_view{ {0, 0}, {0, 0} },
	m_gl_context{ nullptr }
{
};

void render_target::clear(color clearColor) {
    glCheck(bind_framebuffer());
    glCheck(glClearColor(
            static_cast<float>(clearColor.r) / 255.f,
            static_cast<float>(clearColor.g) / 255.f,
            static_cast<float>(clearColor.b) / 255.f,
            static_cast<float>(clearColor.a) / 255.f));

    glCheck(glClear(GL_COLOR_BUFFER_BIT));
	m_just_cleared = true;
}

view render_target::get_view() const {
	return m_view;
}

view render_target::get_default_view() const {
	return view{{0, 0}, size() };
}

void render_target::set_default_view() {
	set_view(get_default_view());
}

void render_target::set_view(const view& n_view) {
	m_view = n_view;

    auto viewport = m_view.get_viewport();
    glCheck(glViewport(
        viewport.left,
        viewport.top,
        viewport.width,
        viewport.height));
}
// ------------------------------------------------------

void render_target::draw(const draw_call& draw) {

}

// ------------------------------------------------------

glm::fvec2 render_target::coord_to_world_pos(glm::ivec2 t_window_coord) {
    const view& v = get_view();
    glm::fvec2 vsize = v.get_size();

    auto vp = v.get_viewport();
    glm::fvec2 vzoom = glm::fvec2(vp.width / vsize.x, vp.height / vsize.y);
    glm::fvec2 viewcenter{ vp.left + vp.width / 2, vp.top + vp.height / 2 };

    glm::fvec2 world_coord = (glm::fvec2{ t_window_coord } - viewcenter) / vzoom;
    world_coord += v.get_center();

    return world_coord;
}

glm::ivec2 render_target::world_pos_to_coord(glm::fvec2 t_world_coord) {
    const view& v = get_view();
    glm::fvec2 vsize = v.get_size();

    auto vp = v.get_viewport();
    glm::fvec2 vzoom = glm::fvec2(vp.width / vsize.x, vp.height / vsize.y);
    glm::fvec2 viewcenter{ vp.left + vp.width / 2, vp.left + vp.height / 2 };

    glm::fvec2 world_coord{ t_world_coord };
    world_coord -= v.get_center();
    world_coord *= vzoom;
    world_coord += viewcenter;

    return glm::ivec2{ roundf(world_coord.x), roundf(world_coord.y) };
}

// ------------------------------------------------------

void render_target::bind_framebuffer() const {
	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer));
}

}

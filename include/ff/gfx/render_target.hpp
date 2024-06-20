#pragma once

#include "ff/gfx/color.hpp"
#include "ff/gfx/camera.hpp"
#include "ff/gfx/draw_call.hpp"

#include <optional>

namespace ff {

class render_target {
public:
    render_target();
    virtual ~render_target() = default;

    virtual glm::ivec2 size() const = 0;

    void clear(color t_color = color::black);

    camera get_view() const;
    virtual camera get_default_view() const;

    void set_view(const camera& t_view);
    void set_default_view();

    void draw(const draw_call& t_draw_call);

    size_t get_vertex_draw_count() { return m_vertex_draw_counter; }
    void reset_vertex_draw_count() { m_vertex_draw_counter = 0; }

    size_t get_draw_call_count() { return m_draw_call_counter; }
    void reset_draw_call_count() { m_draw_call_counter = 0; }

    glm::vec2 coord_to_world_pos(glm::ivec2 t_window_coord);
    glm::ivec2 world_pos_to_coord(glm::vec2 t_world_coord);

protected:
    bool m_just_cleared = true;
    camera m_view;
    void* m_gl_context;
    unsigned m_framebuffer = 0; // default framebuffer
    std::optional<draw_call> m_prev_draw_call;

private:
    void bind_framebuffer() const;

    size_t m_vertex_draw_counter = 0;
    size_t m_draw_call_counter = 0;
};

}
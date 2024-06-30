#pragma once

#include "ff/gfx/color.hpp"
#include "ff/gfx/view.hpp"
#include "ff/gfx/draw_call.hpp"

#include <optional>

namespace ff {

class render_target {
public:
    render_target();
    render_target(const render_target&) = delete;
    render_target& operator=(const render_target&) = delete;
    render_target(render_target&&) noexcept;
    render_target& operator=(render_target&&) noexcept;
    virtual ~render_target() = default;

    virtual ivec2 size() const = 0;

    void clear(color t_color = color::black);

    view get_view() const;
    virtual view get_default_view() const;

    void set_view(const view& t_view);
    void set_default_view();

    void draw(const draw_call& t_draw_call);

    // size_t get_vertex_draw_count() { return m_vertex_draw_counter; }
    // void reset_vertex_draw_count() { m_vertex_draw_counter = 0; }

    // size_t get_draw_call_count() { return m_draw_call_counter; }
    // void reset_draw_call_count() { m_draw_call_counter = 0; }

    vec2 coord_to_world_pos(ivec2 t_window_coord);
    ivec2 world_pos_to_coord(vec2 t_world_coord);

protected:
    bool m_just_cleared = true;
    view m_view;
    void* m_gl_context;
    unsigned m_framebuffer = 0; // default framebuffer
    // std::optional<draw_call> m_prev_draw_call;

private:
    void bind_framebuffer() const;

    // size_t m_vertex_draw_counter = 0;
    // size_t m_draw_call_counter = 0;
};

}

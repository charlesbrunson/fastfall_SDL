#pragma once

#include "ff/gfx/color.hpp"
#include "ff/gfx/view.hpp"
#include "ff/gfx/draw_call.hpp"

#include <optional>

namespace ff {

class render_target {
public:
    render_target();
    virtual ~render_target() = default;

    virtual vec2i get_size() const = 0;

    void clear(color clearColor = color::black);

    view get_view() const;
    virtual view get_default_view() const;

    void set_view(const view& view);
    void set_default_view();

    void draw(const draw_call& draw);

    size_t get_vertex_draw_count() { return m_vertex_draw_counter; }
    void reset_vertex_draw_count() { m_vertex_draw_counter = 0; }

    size_t get_draw_call_count() { return m_draw_call_counter; }
    void reset_draw_call_count() { m_draw_call_counter = 0; }

    vec2f coord_to_world_pos(int windowCoordX, int windowCoordY);
    vec2f coord_to_world_pos(vec2i windowCoord);
    vec2i world_pos_to_coord(float worldCoordX, float worldCoordY);
    vec2i world_pos_to_coord(vec2f worldCoord);

protected:
    std::optional<draw_call> m_prev_draw_call;

    bool m_just_cleared = true;
    view m_view;

    void* m_gl_context;
    unsigned m_framebuffer = 0; // default framebuffer

private:
    void bind_framebuffer() const;

    size_t m_vertex_draw_counter = 0;
    size_t m_draw_call_counter = 0;
};

}

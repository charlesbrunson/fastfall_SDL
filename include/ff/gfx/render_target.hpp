#pragma once

#include "ff/gfx/color.hpp"
#include "ff/gfx/view.hpp"
#include "ff/gfx/draw_call.hpp"
// #include "ff/gfx/render_state.hpp"

#include <optional>

namespace ff {

class Drawable;
class VertexArray;
class TileArray;
class Text;

namespace debug::detail {
struct state_t;
struct gpu_state_t;
}

class RenderTarget {
public:
    RenderTarget();
    virtual ~RenderTarget() = default;

    virtual glm::ivec2 getSize() const = 0;

    // SDL_GLContext getSDLContext() const;
    void clear(color clearColor = color::black);

    view getView() const;
    virtual view getDefaultView() const;

    void setView(const view& view);
    void setDefaultView();

    //void draw(const Drawable& drawable, const RenderState& state = RenderState());
    //void draw(const VertexArray& varray, const RenderState& state = RenderState());
    //void draw(const TileArray& varray, RenderState state = RenderState());
    //void draw(const Text& text, RenderState state = RenderState());
    //void draw(debug::detail::state_t& debug, debug::detail::gpu_state_t& gl, RenderState state = RenderState());

    void draw(const draw_call& draw);

    size_t getVertexCounter() { return vertex_draw_counter; }
    void resetVertexCounter() { vertex_draw_counter = 0; }

    size_t getDrawCallCounter() { return draw_call_counter; }
    void resetDrawCallCounter() { draw_call_counter = 0; }

    glm::fvec2 coordToWorldPos(int windowCoordX, int windowCoordY);
    glm::fvec2 coordToWorldPos(glm::ivec2 windowCoord);
    glm::ivec2 worldPosToCoord(float worldCoordX, float worldCoordY);
    glm::ivec2 worldPosToCoord(glm::fvec2 worldCoord);

protected:
    bool hasShader = false;
    bool hasBlend = false;
    std::optional<draw_info> previousRender;

    bool justCleared = true;
    view m_view;

    SDL_GLContext m_context;
    GLuint m_FBO = 0; // default framebuffer

private:
    void bindFramebuffer() const;

    // void applyBlend(const BlendMode& blend) const;
    // void applyShader(const ShaderProgram* shader) const;
    // void applyUniforms(const Transform& transform, const RenderState& state) const;
    // void applyTexture(const TextureRef& texture) const;

    size_t vertex_draw_counter = 0;
    size_t draw_call_counter = 0;
};

}

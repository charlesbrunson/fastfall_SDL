#pragma once

#include "Color.hpp"
#include "View.hpp"
#include "RenderState.hpp"
//#include "VertexArray.hpp"
//#include "TileArray.hpp"

#include "opengl.hpp"

#include <optional>

namespace ff {

class Drawable;
class VertexArray;
class TileArray;
class Text;

class RenderTarget {
public:
	RenderTarget();
	virtual ~RenderTarget() = default;

	virtual glm::ivec2 getSize() const = 0;

	SDL_GLContext getSDLContext() const;
	void clear(Color clearColor = Color::Black);

	View getView() const;
	virtual View getDefaultView() const;

	void setView(const View& view);
	void setDefaultView();

	void draw(const Drawable& drawable, const RenderState& state = RenderState());
	void draw(const VertexArray& varray, const RenderState& state = RenderState());
	void draw(const TileArray& varray, RenderState state = RenderState());
	void draw(const Text& text, RenderState state = RenderState());

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
	std::optional<RenderState> previousRender;

	View m_view;
	SDL_GLContext m_context;

	GLuint m_FBO = 0; // default framebuffer

	bool justCleared = true;


private:
	void bindFramebuffer() const;

	void applyBlend(const BlendMode& blend) const;
	void applyShader(const ShaderProgram* shader) const;
	void applyUniforms(const Transform& transform, const RenderState& state) const;
	void applyTexture(const TextureRef& texture) const;

	size_t vertex_draw_counter = 0;
	size_t draw_call_counter = 0;
};

}

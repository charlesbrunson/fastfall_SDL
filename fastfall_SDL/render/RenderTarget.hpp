#pragma once

#include "Color.hpp"
#include "View.hpp"
#include "RenderState.hpp"
#include "VertexArray.hpp"
#include "shapes/ShapeLine.hpp"
#include "shapes/ShapeRectangle.hpp"
#include "shapes/ShapeCircle.hpp"

namespace ff {

class Drawable;

class RenderTarget {
public:
	RenderTarget()
		: m_view{ {0, 0}, {0, 0} },
		m_context{ nullptr }
	{
		
	};
	virtual ~RenderTarget() = default;

	virtual glm::ivec2 getSize() const = 0;

	SDL_GLContext getSDLContext() const;
	void clear(Color clearColor = Color::Black);

	View getView() const;
	virtual View getDefaultView() const;

	void setView(const View& view);
	void setDefaultView();

	//virtual void setActive() = 0;

	void draw(const Drawable& drawable, const RenderState& state = RenderState::Default);
	void draw(const VertexArray& varray, const RenderState& state = RenderState::Default);
	//void draw(const ShapeLine& line, const RenderState& state = RenderState::Default);
	//void draw(const ShapeRectange& line, const RenderState& state = RenderState::Default);
	//void draw(const ShapeCircle& line, const RenderState& state = RenderState::Default);

protected:
	View m_view;
	SDL_GLContext m_context;

	GLuint m_FBO = 0; // default framebuffer

private:
	void bindFramebuffer() const;

	void applyBlend(const BlendMode& blend) const;
	void applyShader(const ShaderProgram* shader) const;
	void applyUniforms(const Transform& transform, const RenderState& state) const;
	void applyTexture(const TextureRef& texture) const;
};

}
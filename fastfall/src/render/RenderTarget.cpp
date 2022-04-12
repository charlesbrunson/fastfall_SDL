#include "fastfall/render/RenderTarget.hpp"

#include "fastfall/render/Drawable.hpp"
#include "fastfall/render/VertexArray.hpp"
#include "fastfall/render/TileArray.hpp"
#include "fastfall/render/Text.hpp"

#include "detail/error.hpp"

namespace ff {

RenderTarget::RenderTarget()
	: m_view{ {0, 0}, {0, 0} },
	m_context{ nullptr }
{

};

void RenderTarget::clear(Color clearColor) {
	bindFramebuffer();
	auto clamped = glm::fvec4{ clearColor.toVec4() } / 255.f;
	glClearColor(clamped[0], clamped[1], clamped[2], clamped[3]);
	glClear(GL_COLOR_BUFFER_BIT);
	justCleared = true;

	resetVertexCounter();
	resetDrawCallCounter();
}


SDL_GLContext RenderTarget::getSDLContext() const {
	return m_context;
}

View RenderTarget::getView() const {
	return m_view;
}
View RenderTarget::getDefaultView() const {
	return View{ {0, 0}, getSize() };
}

void RenderTarget::setDefaultView() {
	setView(getDefaultView());
}

void RenderTarget::setView(const View& view) {
	m_view = view;
	glViewport(
		m_view.getViewport()[0],
		m_view.getViewport()[1],
		m_view.getViewport()[2],
		m_view.getViewport()[3]);

}

void RenderTarget::draw(const Drawable& drawable, const RenderState& state) {
	drawable.draw(*this, state);
}

void RenderTarget::draw(const VertexArray& varray, const RenderState& state) {

	if (varray.size() == 0)
		return;

	varray.glTransfer();

	bindFramebuffer();

	if (!previousRender) {
		previousRender = RenderState{};
	}

	if (state.blend != previousRender->blend || !hasBlend) {
		applyBlend(state.blend);
		hasBlend = true;
	}

	if (state.program != previousRender->program || !hasShader) {
		applyShader(state.program);
		hasShader = (state.program != nullptr);
	}

	if (state.program) {
		applyUniforms(Transform::combine(varray.getTransform(), state.transform), state);
	}

	if (state.texture.get()->getID() != previousRender->texture.get()->getID() || justCleared) {
		applyTexture(state.texture);
	}

	glCheck(glBindVertexArray(varray.gl.m_array));
	glCheck(glDrawArrays(static_cast<GLenum>(varray.m_primitive), 0, varray.size()));

	vertex_draw_counter += varray.size();
	draw_call_counter++;

	previousRender = state;
	justCleared = false;
}
void RenderTarget::draw(const TileArray& tarray, RenderState state) {
	if (tarray.tile_count == 0)
		return;

	state.transform = Transform::combine(state.transform, Transform(tarray.offset));
	state.texture = tarray.m_tex;
	state.program = &ShaderProgram::getTileArrayProgram();

	tarray.glTransfer();

	bindFramebuffer();

	if (!previousRender) {
		previousRender = RenderState{};
	}

	if (state.blend != previousRender->blend || !hasBlend) {
		applyBlend(state.blend);
		hasBlend = true;
	}

	//LOG_INFO("using shader: {}", state.program ? state.program->getID() : -1);
	if (state.program != previousRender->program || !hasShader) {
		applyShader(state.program);
		hasShader = (state.program != nullptr);
	}

	if (state.program) {
		applyUniforms(Transform::combine(tarray.getTransform(), state.transform), state);
		int columns_uniform_id = state.program->getOtherUniformID("columns");
		if (columns_uniform_id >= 0) {
			glCheck(glUniform1ui(columns_uniform_id, tarray.m_size.x));
		}
	}

	if (state.texture.get()->getID() != previousRender->texture.get()->getID() || justCleared) {
		applyTexture(state.texture);
	}

	glCheck(glBindVertexArray(tarray.gl.m_array));
	glCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, tarray.tiles.size()));

	vertex_draw_counter += tarray.tiles.size() * 2;
	draw_call_counter++;

	previousRender = state;
	justCleared = false;
}


void RenderTarget::draw(const Text& text, RenderState state) {
	if (text.gl_text.size() == 0)
		return;

	if (text.m_font && !text.bitmap_texture.exists())
	{
		text.m_font->loadBitmapTex(text.px_size);
	}

	state.texture = text.bitmap_texture;
	state.program = &ShaderProgram::getTextProgram();

	text.glTransfer();

	if (!previousRender) {
		previousRender = RenderState{};
	}

	if (state.blend != previousRender->blend || !hasBlend) {
		applyBlend(state.blend);
		hasBlend = true;
	}

	if (state.program != previousRender->program || !hasShader) {
		applyShader(state.program);
		hasShader = (state.program != nullptr);
	}

	if (state.program) {
		applyUniforms(Transform::combine(text.getTransform(), state.transform), state);
		int char_size_uni_id = state.program->getOtherUniformID("char_size");
		if (char_size_uni_id >= 0) {
			Vec2f size { text.getFont()->getGlyphSize() };
			glCheck(glUniform2f(char_size_uni_id, size.x, size.y));
		}
	}

	if (state.texture.get()->getID() != previousRender->texture.get()->getID() || justCleared) {
		applyTexture(state.texture);
	}

	glCheck(glBindVertexArray(text.gl.m_array));
	glCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, text.gl_text.size()));

	vertex_draw_counter += text.gl_text.size() * 2;
	draw_call_counter++;

	previousRender = state;
	justCleared = false;
}

// ------------------------------------------------------

void RenderTarget::bindFramebuffer() const {
	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
}

void RenderTarget::applyBlend(const BlendMode& blend) const {
	glCheck(glEnable(GL_BLEND));

	if (blend.hasConstantColor()) {
		glm::fvec4 clamped = glm::fvec4{ blend.getColor().toVec4() } / 255.f;
		glCheck(glBlendColor(
			clamped[0],
			clamped[1],
			clamped[2],
			clamped[3]
		));
	}

	if (blend.hasSeparateFactor()) {
		glCheck(glBlendFuncSeparate(
			static_cast<GLenum>(blend.getSrcFactorRGB()),
			static_cast<GLenum>(blend.getDstFactorRGB()),
			static_cast<GLenum>(blend.getSrcFactorA()),
			static_cast<GLenum>(blend.getDstFactorA())
		));
	}
	else {
		glCheck(glBlendFunc(
			static_cast<GLenum>(blend.getSrcFactorRGB()),
			static_cast<GLenum>(blend.getDstFactorRGB())
		));
	}

	if (blend.hasSeparateEquation()) {
		glCheck(glBlendEquationSeparate(
			static_cast<GLenum>(blend.getEquationRGB()),
			static_cast<GLenum>(blend.getEquationA())
		));
	}
	else {
		glCheck(glBlendEquation(
			static_cast<GLenum>(blend.getEquationRGB())
		));
	}

}

void RenderTarget::applyShader(const ShaderProgram* shader) const {

	if (shader != nullptr) {
		shader->use();
	}
	else {
		glCheck(glUseProgram(0));
	}
}

void RenderTarget::applyUniforms(const Transform& transform, const RenderState& state) const {
	if (state.program->getMdlUniformID()  >= 0) {
		glCheck(glUniformMatrix3fv(state.program->getMdlUniformID(), 1, GL_FALSE, glm::value_ptr(transform.getMatrix())));
	}
	if (state.program->getViewUniformID() >= 0) {
		glm::mat3 viewmat = m_view.getMatrix();
		glCheck(glUniformMatrix3fv(state.program->getViewUniformID(), 1, GL_FALSE, glm::value_ptr(viewmat)));
	}
}

void RenderTarget::applyTexture(const TextureRef& texture) const {
	texture.bind();
}

}

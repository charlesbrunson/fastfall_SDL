#include "fastfall/render/RenderTarget.hpp"

#include "fastfall/render/Drawable.hpp"
#include "fastfall/render/VertexArray.hpp"

namespace ff {

void RenderTarget::clear(Color clearColor) {
	bindFramebuffer();
	auto clamped = glm::fvec4{ clearColor.toVec4() } / 255.f;
	glClearColor(clamped[0], clamped[1], clamped[2], clamped[3]);
	glClear(GL_COLOR_BUFFER_BIT);
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

	varray.glTransfer();

	bindFramebuffer();
	applyBlend(state.blend);
	applyShader(state.program);
	applyUniforms(Transform::combine(varray.getTransform(), state.transform), state);
	applyTexture(state.texture);

	glBindVertexArray(varray.gl.m_array);
	glDrawArrays(static_cast<GLenum>(varray.m_primitive), 0, varray.size());
}

// ------------------------------------------------------

void RenderTarget::bindFramebuffer() const {
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
}

void RenderTarget::applyBlend(const BlendMode& blend) const {
	glEnable(GL_BLEND);

	if (blend.hasConstantColor()) {
		glm::fvec4 clamped = glm::fvec4{ blend.getColor().toVec4() } / 255.f;
		glBlendColor(
			clamped[0],
			clamped[1],
			clamped[2],
			clamped[3]
		);
	}

	if (blend.hasSeparateFactor()) {
		glBlendFuncSeparate(
			static_cast<GLenum>(blend.getSrcFactorRGB()),
			static_cast<GLenum>(blend.getDstFactorRGB()),
			static_cast<GLenum>(blend.getSrcFactorA()),
			static_cast<GLenum>(blend.getDstFactorA())
		);
	}
	else {
		glBlendFunc(
			static_cast<GLenum>(blend.getSrcFactorRGB()),
			static_cast<GLenum>(blend.getDstFactorRGB())
		);
	}

	if (blend.hasSeparateEquation()) {
		glBlendEquationSeparate(
			static_cast<GLenum>(blend.getEquationRGB()),
			static_cast<GLenum>(blend.getEquationA())
		);
	}
	else {
		glBlendEquation(
			static_cast<GLenum>(blend.getEquationRGB())
		);
	}

}

void RenderTarget::applyShader(const ShaderProgram* shader) const {

	if (shader != nullptr) {
		shader->use();
	}
	else {
		glUseProgram(0);
	}
}

void RenderTarget::applyUniforms(const Transform& transform, const RenderState& state) const {

	if (!state.program) {
		return;
	}

	if (state.program->getMdlUniformID()  >= 0) {
		glUniformMatrix3fv(state.program->getMdlUniformID(), 1, GL_FALSE, glm::value_ptr(transform.getMatrix()));
	}
	if (state.program->getViewUniformID() >= 0) {
		glm::mat3 viewmat = m_view.getMatrix();
		glUniformMatrix3fv(state.program->getViewUniformID(), 1, GL_FALSE, glm::value_ptr(viewmat));
	}
}

void RenderTarget::applyTexture(const TextureRef& texture) const {
	texture.bind();
}

}
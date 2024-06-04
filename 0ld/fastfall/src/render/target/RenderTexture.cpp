
#include "fastfall/render/target/RenderTexture.hpp"

namespace ff {

bool RenderTexture::create(glm::ivec2 size) {
	return create(size.x, size.y);
}
bool RenderTexture::create(int sizeX, int sizeY) {
	if (m_valid) {
		glDeleteFramebuffers(1, &m_FBO);
	}
	m_size.x = sizeX;
	m_size.y = sizeY;

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	m_tex.create(m_size.x, m_size.y);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex.getID(), 0);

	m_valid = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (m_valid) {
		setDefaultView();
	}
	return m_valid;
}

View RenderTexture::getDefaultView() const {
	return View{ {0, 0}, getSize(), {1.f, -1.f} };
}

const Texture* RenderTexture::getTexture() const {
	return &m_tex;
}

}
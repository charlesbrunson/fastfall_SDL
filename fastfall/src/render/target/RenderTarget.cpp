#include "fastfall/render/target/RenderTarget.hpp"

#include "fastfall/render/drawable/Drawable.hpp"
#include "fastfall/render/drawable/VertexArray.hpp"
#include "fastfall/render/drawable/TileArray.hpp"
#include "fastfall/render/drawable/Text.hpp"
#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/resource/Resources.hpp"

#include "../detail/error.hpp"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

namespace ff {

RenderTarget::RenderTarget()
	: m_view{ {0, 0}, {0, 0} },
	m_context{ nullptr }
{

};

void RenderTarget::clear(Color clearColor) {
    ZoneScoped;
    TracyGpuZone("RenderTarget::Clear");

    glCheck(bindFramebuffer());
    glCheck(glClearColor(
            static_cast<float>(clearColor.r) / 255.f,
            static_cast<float>(clearColor.g) / 255.f,
            static_cast<float>(clearColor.b) / 255.f,
            static_cast<float>(clearColor.a) / 255.f));

    glCheck(glClear(GL_COLOR_BUFFER_BIT));
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
	Rectf vp = m_view.getViewport();
    glCheck(glViewport(
    	vp.left,
    	vp.top,
    	vp.width,
    	vp.height
	));
}

void RenderTarget::draw(const Drawable& drawable, const RenderState& state) {
 	if (drawable.visible) {
		drawable.draw(*this, state);
	}
}

void RenderTarget::draw(const VertexArray& varray, const RenderState& state) {
    ZoneScoped;
    TracyGpuZone("RenderTarget::Draw Vertex Array");

	if (varray.size() == 0)
		return;

	varray.glTransfer();

    if (varray.gl.m_array == 0)
        return;

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
    ZoneScoped;
    TracyGpuZone("RenderTarget::Draw TileArray");
	if (tarray.tile_count == 0)
		return;

	state.transform = Transform::combine(state.transform, Transform(tarray.offset));
	state.texture = tarray.m_tex;
    constexpr std::string_view shader = "tile.glsl";
    if (auto ptr = Resources::get<ShaderAsset>(shader)) {
        state.program = &ptr->getProgram();
    }
    else {
        LOG_ERR_("failed to render, {} shader not loaded", shader);
        return;
    }

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
		auto columns = state.program->getUniform("columns");
		if (columns) {
			glCheck(glUniform1ui(columns->loc, tarray.m_size.x));
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
    ZoneScoped;
    TracyGpuZone("RenderTarget::Draw Text");
	if (text.gl_text.size() == 0)
		return;

	if (text.m_font && !text.bitmap_texture.exists())
	{
		text.m_font->loadBitmapTex(text.px_size);
	}

	state.texture = text.bitmap_texture;
    constexpr std::string_view shader = "text.glsl";
    if (auto ptr = Resources::get<ShaderAsset>(shader)) {
        state.program = &ptr->getProgram();
    }
    else {
        LOG_ERR_("failed to render, {} shader not loaded", shader);
        return;
    }

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
		auto char_size_uni = state.program->getUniform("char_size");
		if (char_size_uni) {
			Vec2f size { text.getFont()->getGlyphSize() };
			glCheck(glUniform2f(char_size_uni->loc, size.x, size.y));
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

void RenderTarget::draw(debug::detail::state_t& debug, debug::detail::gpu_state_t& gl, RenderState state) {
    ZoneScoped;
    TracyGpuZone("RenderTarget::Draw Debug");
    if (debug.vertices.empty())
        return;

    if (gl.m_array == 0)
        return;

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

    glCheck(glBindVertexArray(gl.m_array));

    for (auto& call : debug.compressed_calls) {
        if (state.program) {
            applyUniforms(Transform::combine(Transform{}.translate(call.draw_offset), state.transform), state);
        }
        glCheck(glDrawArrays(static_cast<GLenum>(call.primitive), call.vertex_offset, call.vertex_count));
    }

    vertex_draw_counter += debug.vertices.size();
    draw_call_counter++;

    previousRender = state;
    justCleared = false;
}

// ------------------------------------------------------

Vec2f RenderTarget::coordToWorldPos(Vec2i windowCoord) {
    return coordToWorldPos(windowCoord.x, windowCoord.y);
}

Vec2i RenderTarget::worldPosToCoord(Vec2f worldCoord) {
    return worldPosToCoord(worldCoord.x, worldCoord.y);
}

Vec2f RenderTarget::coordToWorldPos(int windowCoordX, int windowCoordY) {
    const View& v = getView();
    Vec2f vsize = v.getSize();
	auto vp = v.getViewport();
    Vec2f vzoom = vp.getSize() / vsize;
    Vec2f viewcenter = vp.center();

    Vec2f world_coord{
            ((float)windowCoordX - viewcenter.x) / vzoom.x,
            ((float)windowCoordY - viewcenter.y) / vzoom.y
    };
    world_coord += v.getCenter();

    return world_coord;
}

Vec2i RenderTarget::worldPosToCoord(float worldCoordX, float worldCoordY) {
    const View& v = getView();
    Vec2f vsize = v.getSize();
	auto vp = v.getViewport();
    Vec2f vzoom = vp.getSize() / vsize;
    Vec2f viewcenter = vp.center();

    Vec2f world_coord{ worldCoordX, worldCoordY };
    world_coord -= v.getCenter();
    world_coord *= vzoom;
    world_coord += viewcenter;

    return Vec2i{ roundf(world_coord.x), roundf(world_coord.y) };
}
// ------------------------------------------------------

void RenderTarget::bindFramebuffer() const {
	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
}

void RenderTarget::applyBlend(const BlendMode& blend) const {
	glCheck(glEnable(GL_BLEND));

	if (blend.hasConstantColor()) {
        auto blend_color = blend.getColor();
        glCheck(glClearColor(
            static_cast<float>(blend_color.r) / 255.f,
            static_cast<float>(blend_color.g) / 255.f,
            static_cast<float>(blend_color.b) / 255.f,
            static_cast<float>(blend_color.a) / 255.f));
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
	if (auto uni = state.program->getUniform("model")) {
		glCheck(glUniformMatrix3fv(uni->loc, 1, GL_FALSE, glm::value_ptr(transform.getMatrix())));
	}
	if (auto uni = state.program->getUniform("view")) {
        auto mat = m_view.getMatrix();
		glCheck(glUniformMatrix3fv(uni->loc, 1, GL_FALSE, glm::value_ptr(mat)));
	}
}

void RenderTarget::applyTexture(const TextureRef& texture) const {
	texture.bind();
}

}

#pragma once

#include "fastfall/render/external/opengl.hpp"

namespace ff::render {
	bool init();
	void quit();
	bool is_init();

	bool glew_init();
	bool glew_is_init();
}

#include "fastfall/render/target/Window.hpp"
#include "fastfall/render/util/Color.hpp"
#include "fastfall/render/util/Vertex.hpp"
#include "fastfall/render/drawable/VertexArray.hpp"
#include "fastfall/render/util/Texture.hpp"
#include "fastfall/render/target/RenderTexture.hpp"
#include "fastfall/render/drawable/Sprite.hpp"
#include "fastfall/render/util/Shader.hpp"
#include "fastfall/render/util/Texture.hpp"
#include "fastfall/render/util/RenderState.hpp"
#include "fastfall/render/drawable/ShapeLine.hpp"
#include "fastfall/render/drawable/ShapeCircle.hpp"
#include "fastfall/render/drawable/ShapeRectangle.hpp"
#include "fastfall/render/util/Font.hpp"
#include "fastfall/render/drawable/Text.hpp"

namespace ff {
	void ImGuiNewFrame(Window& window);
	void ImGuiRender();
}

namespace ff {
	void glStaleVertexArrays(size_t count, const GLuint* vao);
	void glStaleVertexBuffers(size_t count, const GLuint* vbo);
	void glStaleVertexArrays(const GLuint vao);
	void glStaleVertexBuffers(const GLuint vbo);
	void glDeleteStale();
}

#pragma once

#include "fastfall/render/opengl.hpp"


namespace ff {
	
	bool FFinit();

	void FFquit();

	bool FFisInit();

	void FFinitGLEW();
	bool FFisGLEWInit();

}


#include "fastfall/render/Window.hpp"
#include "fastfall/render/Color.hpp"
#include "fastfall/render/Vertex.hpp"
#include "fastfall/render/VertexArray.hpp"
#include "fastfall/render/Texture.hpp"
#include "fastfall/render/RenderTexture.hpp"
#include "fastfall/render/Sprite.hpp"
#include "fastfall/render/Shader.hpp"
#include "fastfall/render/Texture.hpp"
#include "fastfall/render/RenderState.hpp"
#include "fastfall/render/ShapeLine.hpp"
#include "fastfall/render/ShapeCircle.hpp"
#include "fastfall/render/ShapeRectangle.hpp"
#include "fastfall/render/Font.hpp"
#include "fastfall/render/Text.hpp"

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

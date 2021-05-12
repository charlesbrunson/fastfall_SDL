#pragma once

#include "fastfall/render/opengl.hpp"


namespace ff {
	
	bool init();

	void quit();

	bool isInit();

	void initGLEW();
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


namespace ff {
	void ImGuiNewFrame(Window& window);
	void ImGuiRender();
}
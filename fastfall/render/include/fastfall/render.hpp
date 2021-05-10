#pragma once

#include "render/opengl.hpp"


namespace ff {
	
	bool init();

	void quit();

	bool isInit();

	void initGLEW();
}

#include "render/Window.hpp"
#include "render/Color.hpp"
#include "render/Vertex.hpp"
#include "render/VertexArray.hpp"
#include "render/Texture.hpp"
#include "render/RenderTexture.hpp"
#include "render/Sprite.hpp"
#include "render/Shader.hpp"
#include "render/Texture.hpp"
#include "render/RenderState.hpp"
#include "render/ShapeLine.hpp"
#include "render/ShapeCircle.hpp"
#include "render/ShapeRectangle.hpp"


namespace ff {
	void ImGuiNewFrame(Window& window);
	void ImGuiRender();
}
#pragma once

#include <SDL.h>

namespace ff {
	
	bool init();

	void quit();

	bool isInit();

	void initGLEW();

}

#include "Color.hpp"
#include "Window.hpp"
#include "Vertex.hpp"
#include "VertexArray.hpp"
#include "Texture.hpp"
#include "RenderTexture.hpp"
#include "Sprite.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "RenderState.hpp"
#include "shapes/ShapeLine.hpp"
#include "shapes/ShapeCircle.hpp"
#include "shapes/ShapeRectangle.hpp"

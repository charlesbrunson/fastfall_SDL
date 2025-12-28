#pragma once 

#include "fastfall/util/glm_types.hpp"
#include "Color.hpp"

namespace ff {

struct Vertex {
	Vec2f pos;
	Color color;
	Vec2f tex_pos;
};

}
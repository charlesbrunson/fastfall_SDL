#pragma once 

#include <glm/glm.hpp>

#include "Color.hpp"

namespace ff {

struct Vertex {
	glm::fvec2 pos;
	ff::Color color;
	glm::fvec2 tex_pos;
};

}
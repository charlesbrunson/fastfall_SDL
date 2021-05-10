#include "render/Color.hpp"

namespace ff {

const Color Color::Transparent	{ 0x00000000 };
const Color Color::White		{ 0xFFFFFFFF };
const Color Color::Black		{ 0x000000FF };
const Color Color::Red			{ 0xFF0000FF };
const Color Color::Green		{ 0x00FF00FF };
const Color Color::Blue			{ 0x0000FFFF };
const Color Color::Yellow		{ 0xFFFF00FF };
const Color Color::Cyan			{ 0x00FFFFFF };
const Color Color::Magenta		{ 0xFF00FFFF };

glm::uvec4 Color::toVec4() const {
	return glm::fvec4{ r,g,b,a };
}

}
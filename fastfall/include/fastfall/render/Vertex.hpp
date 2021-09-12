#pragma once 

#include <glm/glm.hpp>

#include "Color.hpp"

namespace ff {

struct Vertex {



	/*
	Vertex()
		: pos{ 0.f, 0.f },
		tex_pos{ 0.f, 0.f },
		color{ ff::Color::White }
	{

	}

	Vertex(glm::fvec2 _pos, ff::Color _color = ff::Color::White, glm::fvec2 _tex_pos = { 0.f, 0.f })
		: pos{ _pos },
		tex_pos{ _tex_pos },
		color{ _color }
	{

	}
	*/

	glm::fvec2 pos;
	ff::Color color;
	glm::fvec2 tex_pos;

};

}
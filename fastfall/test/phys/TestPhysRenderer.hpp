#pragma once

#include <SDL.h>
#include <fmt/format.h>

#include "fastfall/game/Instance.hpp"

#define FF_TESTPHYSRENDERER_ENABLED 1

class TestPhysRenderer {
public:
	TestPhysRenderer(ff::Rectf area)
		: render_area(area)
	{
	}

	ff::Rectf render_area;
	size_t curr_frame = 0;

	void render(ff::CollisionManager& colMan);
};
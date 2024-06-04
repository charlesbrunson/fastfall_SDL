#pragma once

#include <memory>
#include <SDL.h>
#include <fmt/format.h>

#include "fastfall/game/World.hpp"

#define FF_TESTPHYSRENDERER_ENABLED 1

class TestPhysRenderer {
public:
	TestPhysRenderer(ff::World& w, ff::Rectf area);
	~TestPhysRenderer();

	TestPhysRenderer(const TestPhysRenderer&) = delete;
	TestPhysRenderer& operator=(const TestPhysRenderer&) = delete;

	ff::Rectf render_area;
	size_t curr_frame = 0;

	unsigned frame_delay = 2;

	void draw();

    ff::World* world;

private:
	static constexpr float scale = 2;

#if FF_TESTPHYSRENDERER_ENABLED
	SDL_Surface* surface = nullptr;
	SDL_Renderer* render = nullptr;

	struct GifWriterImpl;
	std::unique_ptr<GifWriterImpl> impl;
#endif
};

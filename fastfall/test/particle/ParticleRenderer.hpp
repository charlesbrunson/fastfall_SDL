#pragma once

#include <memory>
#include <SDL.h>
#include <fmt/format.h>

#include "fastfall/game/particle/Emitter.hpp"

#define FF_PARTICLERENDERER_ENABLED 1

class ParticleRenderer {
public:
	ParticleRenderer(ff::Emitter& emit, ff::Rectf area);
	~ParticleRenderer();

	ParticleRenderer(const ParticleRenderer&) = delete;
	ParticleRenderer& operator=(const ParticleRenderer&) = delete;

    unsigned frame_delay = 2;
	ff::Rectf render_area;
	size_t curr_frame = 0;

	void draw();


private:
	static constexpr float scale = 2;

    ff::Emitter* emitter;

#if FF_PARTICLERENDERER_ENABLED
	SDL_Surface* surface = nullptr;
	SDL_Renderer* render = nullptr;

	struct GifWriterImpl;
	std::unique_ptr<GifWriterImpl> impl;
#endif
};

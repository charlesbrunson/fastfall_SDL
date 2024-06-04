#include "ParticleRenderer.hpp"

#include "gtest/gtest.h"

#include "fastfall/render/util/Color.hpp"

#if FF_PARTICLERENDERER_ENABLED
#include "gif.h"
#endif

using namespace ff;

#if FF_PARTICLERENDERER_ENABLED
struct ParticleRenderer::GifWriterImpl {
	GifWriter writer;
};
#endif

ParticleRenderer::ParticleRenderer(Emitter& emit, ff::Rectf area)
	: emitter(&emit)
    , render_area(area)
{
#if FF_PARTICLERENDERER_ENABLED
	std::string test_name = fmt::format("particle_render_out/{}__{}.gif",
		::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name(),
		::testing::UnitTest::GetInstance()->current_test_info()->name());

	impl = std::make_unique<GifWriterImpl>();

	surface = SDL_CreateRGBSurfaceWithFormat(0, render_area.width * scale, render_area.height * scale, 32, SDL_PIXELFORMAT_ABGR8888);
	SDL_FillRect(surface, NULL, 0);
	render = SDL_CreateSoftwareRenderer(surface);

	GifBegin(&impl->writer, test_name.c_str(), render_area.width * scale, render_area.height * scale, frame_delay);
#endif
}

ParticleRenderer::~ParticleRenderer()
{
#if FF_PARTICLERENDERER_ENABLED
	GifEnd(&impl->writer);
	SDL_FreeSurface(surface);
	SDL_DestroyRenderer(render);
#endif
}

void ParticleRenderer::draw() {

#if FF_PARTICLERENDERER_ENABLED

	//float scale = 2.f;

	//SDL_FillRect(surface, NULL, 0);

	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	SDL_RenderClear(render);

	//SDL_RenderSetScale(render, 4, 4);

	Vec2f off = -render_area.getPosition();


    Color colors[] = {
        Color::White,
        Color::Red,
        Color::Green,
        Color::Blue,
        Color::Yellow,
        Color::Cyan,
        Color::Magenta,
    };
    size_t color_count = 7;

	auto drawLine = [&](Linef line, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		SDL_SetRenderDrawColor(render, r, g, b, a);
		Vec2f p1, p2;

		p1 = (off + line.p1) * scale;
		p2 = (off + line.p2) * scale;
		SDL_RenderDrawLineF(render, p1.x, p1.y, p2.x, p2.y);
	};

    for (auto& p : emitter->particles) {
        Color& c = colors[p.id % color_count];
        drawLine({p.prev_position, p.position}, c.r, c.g, c.b, c.a);
    }

	GifWriteFrame(&impl->writer, (const uint8_t*)surface->pixels, render_area.width * scale, render_area.height * scale, frame_delay);

#endif

	curr_frame++;

}

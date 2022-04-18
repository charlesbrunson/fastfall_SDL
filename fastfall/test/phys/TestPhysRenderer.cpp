#include "TestPhysRenderer.hpp"

#include "gtest/gtest.h"

#if FF_TESTPHYSRENDERER_ENABLED
#include "gif.h"
#endif

using namespace ff;

#if FF_TESTPHYSRENDERER_ENABLED
struct TestPhysRenderer::GifWriterImpl {
	GifWriter writer;
};
#endif

TestPhysRenderer::TestPhysRenderer(ff::Rectf area)
	: render_area(area)
{
#if FF_TESTPHYSRENDERER_ENABLED
	std::string test_name = fmt::format("phys_render_out/{}__{}.gif",
		::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name(),
		::testing::UnitTest::GetInstance()->current_test_info()->name());

	impl = std::make_unique<GifWriterImpl>();

	surface = SDL_CreateRGBSurfaceWithFormat(0, render_area.width * scale, render_area.height * scale, 32, SDL_PIXELFORMAT_ABGR8888);
	SDL_FillRect(surface, NULL, 0);
	render = SDL_CreateSoftwareRenderer(surface);

	GifBegin(&impl->writer, test_name.c_str(), render_area.width * scale, render_area.height * scale, 1000);
#endif
}

TestPhysRenderer::~TestPhysRenderer()
{
#if FF_TESTPHYSRENDERER_ENABLED
	GifEnd(&impl->writer);
	SDL_FreeSurface(surface);
	SDL_DestroyRenderer(render);
#endif
}

void TestPhysRenderer::draw(CollisionManager& colMan) {

#if FF_TESTPHYSRENDERER_ENABLED

	//float scale = 2.f;

	//SDL_FillRect(surface, NULL, 0);

	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	SDL_RenderClear(render);

	//SDL_RenderSetScale(render, 4, 4);

	auto drawRectOutline = [&](Rectf rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		SDL_SetRenderDrawColor(render, r, g, b, a);
		Vec2f p1, p2;

		p1 = math::rect_topleft(rect) * scale;
		p2 = math::rect_topright(rect) * scale;
		SDL_RenderDrawLineF(render, p1.x, p1.y, p2.x, p2.y);

		p1 = math::rect_topright(rect) * scale;
		p2 = math::rect_botright(rect) * scale;
		SDL_RenderDrawLineF(render, p1.x, p1.y, p2.x, p2.y);

		p1 = math::rect_botright(rect) * scale;
		p2 = math::rect_botleft(rect) * scale;
		SDL_RenderDrawLineF(render, p1.x, p1.y, p2.x, p2.y);

		p1 = math::rect_botleft(rect) * scale;
		p2 = math::rect_topleft(rect) * scale;
		SDL_RenderDrawLineF(render, p1.x, p1.y, p2.x, p2.y);
	};

	// draw colliders
	for (const auto& collider : colMan.get_colliders())
	{
		std::vector<std::pair<Rectf, const ColliderQuad*>> quads;
		collider->getQuads(render_area, quads);

		Vec2f offset = collider->getPosition();

		for (const auto& quad : quads) {

			SDL_SetRenderDrawColor(render, 50, 50, 50, 255);
			for (const auto& line : quad.second->surfaces) {
				if (!line.hasSurface) {
					Linef li = line.collider.surface;
					li.p1 += offset;
					li.p2 += offset;
					li.p1 *= scale;
					li.p2 *= scale;
					SDL_RenderDrawLineF(render, li.p1.x, li.p1.y, li.p2.x, li.p2.y);
				}
			}
			

			SDL_SetRenderDrawColor(render, 200, 200, 200, 255);
			for (const auto& line : quad.second->surfaces) {
				if (line.hasSurface) {
					Linef li = line.collider.surface;
					li.p1 += offset;
					li.p2 += offset;
					li.p1 *= scale;
					li.p2 *= scale;
					SDL_RenderDrawLineF(render, li.p1.x, li.p1.y, li.p2.x, li.p2.y);
				}
			}
		}
	}

	// draw collidables
	for (const auto& collidable : colMan.get_collidables()) {
		SDL_SetRenderDrawColor(render, 0, 150, 0, 255);

		Rectf prev = collidable.collidable.getPrevBox();
		Rectf box = collidable.collidable.getBox();

		drawRectOutline(prev, 0, 50, 0, 255);
		drawRectOutline(box, 0, 255, 0, 255);

		{
			SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
			Vec2f p1 = math::rect_mid(box);
			Vec2f p2 = p1 + collidable.collidable.get_vel() * 0.05f;
			p1 *= scale;
			p2 *= scale;
			SDL_RenderDrawLineF(render, p1.x, p1.y, p2.x, p2.y);
		}

		// draw contacts

		for (const auto& contact : collidable.collidable.get_contacts()) {
			if (contact.hasContact) {

				SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
				Vec2f p1 = contact.position;
				Vec2f p2 = p1 + contact.ortho_normal * contact.separation * 1.f;
				p1 *= scale;
				p2 *= scale;
				SDL_RenderDrawLineF(render, p1.x, p1.y, p2.x, p2.y);


				SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
				p1 = contact.collider.surface.p1;
				p2 = contact.collider.surface.p2;
				p1 *= scale;
				p2 *= scale;
				SDL_RenderDrawLineF(render, p1.x, p1.y, p2.x, p2.y);
				p1 = contact.collider.surface.p1 - contact.collider_normal;
				p2 = contact.collider.surface.p2 - contact.collider_normal;
				p1 *= scale;
				p2 *= scale;
				SDL_RenderDrawLineF(render, p1.x, p1.y, p2.x, p2.y);

			}

		}
	}

	GifWriteFrame(&impl->writer, (const uint8_t*)surface->pixels, render_area.width * scale, render_area.height * scale, 2);

#endif

	curr_frame++;

}
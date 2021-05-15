#include "fastfall/game/phys/collider_coretypes/ColliderQuad.hpp"

//#include "fastfall/game/DebugDraw.hpp"

namespace ff {

// TODO
/*
void debug_impl(ColliderQuad* quad, VertexArray& array, size_t startNdx) {
	static auto normal = [](Line<float> line) -> Vec2f {
		Vec2f v = (line.p2 - line.p1);

		v /= sqrtf(v.x * v.x + v.y * v.y);

		std::swap(v.x, v.y);
		v.y *= -1.f;

		return v;
	};

	constexpr static unsigned colors[4] = {
		0xFF0000FF, // red, north
		0x00FF00FF, // green, east
		0x0000FFFF, // blue, south
		0xFFFF00FF // yellow, west
	};

	for (int i = 0; i < 4; i++) {
		if (const ColliderSurface* s = quad->getSurface((Cardinal)i)) {

			sf::Vector2f n = normal(Linef(Vec2f(s->surface.p1), Vec2f(s->surface.p2)));
			sf::Color c = sf::Color(colors[i]);

			array[startNdx + (i * 4)].color = c;
			array[startNdx + (i * 4)].position = Vec2f(s->surface.p1);

			array[startNdx + (i * 4) + 1].color = c;
			array[startNdx + (i * 4) + 1].position = Vec2f(s->surface.p2);

			c.a = 128;

			array[startNdx + (i * 4) + 2].color = c;
			array[startNdx + (i * 4) + 2].position = s->surface.p2;
			array[startNdx + (i * 4) + 2].position -= n;

			array[startNdx + (i * 4) + 3].color = c;
			array[startNdx + (i * 4) + 3].position = s->surface.p1;
			array[startNdx + (i * 4) + 3].position -= n;
		}
	}
}
*/

void debugDrawQuad(ColliderQuad& quad, Vec2f offset) {

	// TODO
	/*
	if (!debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_COLLIDER))
		return;

	debug_draw::set_offset(offset);

	auto& draw = createDebugDrawable<sf::VertexArray, debug_draw::Type::COLLISION_COLLIDER>(sf::PrimitiveType::Quads, 16);
	debug_impl(&quad, draw, 0);

	debug_draw::set_offset();
	*/

}
void debugDrawQuad(size_t count, ColliderQuad* quad, Vec2f offset) {

	// TODO
	/*
	if (!debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_COLLIDER))
		return;

	debug_draw::set_offset(offset);

	auto& draw = createDebugDrawable<sf::VertexArray, debug_draw::Type::COLLISION_COLLIDER>(sf::PrimitiveType::Quads, count * 16);
	size_t ndx = 0;
	for (size_t i = 0; i < count; ndx++) {
		if (!(quad + ndx)->hasAnySurface()) {
			continue;
		}

		debug_impl(quad + ndx, draw, i * 16);
		i++;
	}
	debug_draw::set_offset();
	*/
}

// -------------------------------------------

ColliderQuad::ColliderQuad() noexcept :
	surfaces{}
{

}

ColliderQuad::ColliderQuad(const SurfaceArray& surfaces) noexcept :
	surfaces{ surfaces }
{

}

ColliderQuad::ColliderQuad(SurfaceArray&& surfaces) noexcept :
	surfaces{ surfaces }
{

}

ColliderQuad::ColliderQuad(const Rectf& shape) noexcept {

	Vec2f points[4] = {
		math::rect_topleft(shape),
		math::rect_topright(shape),
		math::rect_botright(shape),
		math::rect_botleft(shape)
	};

	for (unsigned i = 0; i < 4; i++) {
		surfaces[i].hasSurface = true;

		surfaces[i].collider.g0virtual = false;
		surfaces[i].collider.g3virtual = false;

		surfaces[i].collider.ghostp0 = points[(i - 1) % 4];
		surfaces[i].collider.surface.p1 = points[i];
		surfaces[i].collider.surface.p2 = points[(i + 1) % 4];
		surfaces[i].collider.ghostp3 = points[(i + 2) % 4];

		surfaces[i].collider.prev = &surfaces[(i - 1) % 4].collider;
		surfaces[i].collider.next = &surfaces[(i + 1) % 4].collider;
	}
}

void ColliderQuad::translate(Vec2f offset) noexcept {
	for (auto& i : surfaces) {
		i.collider.surface.p1 += offset;
		i.collider.surface.p2 += offset;
		i.collider.ghostp0 += offset;
		i.collider.ghostp3 += offset;
	}
}

const ColliderSurface* ColliderQuad::getSurface(Cardinal side) const noexcept {
	return surfaces[side].hasSurface ? &surfaces[side].collider : nullptr;
}

ColliderSurface* ColliderQuad::getSurface(Cardinal side) noexcept {
	return surfaces[side].hasSurface ? &surfaces[side].collider : nullptr;
}

void ColliderQuad::setSurface(Cardinal side, const ColliderSurface& surface) noexcept {
	surfaces[side].hasSurface = true;
	surfaces[side].collider = surface;
}

void ColliderQuad::removeSurface(Cardinal side) {
	surfaces[side].hasSurface = false;
}

}
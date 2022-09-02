#include "fastfall/game/phys/collider_coretypes/ColliderQuad.hpp"

#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/render/VertexArray.hpp"

#include "fastfall/util/log.hpp"

namespace ff {


constexpr void debug_impl(ColliderQuad* quad, VertexArray& array, size_t startNdx) {
	constexpr auto normal = [](Line<float> line) -> Vec2f {
		Vec2f v = (line.p2 - line.p1);

		v /= sqrtf(v.x * v.x + v.y * v.y);

		std::swap(v.x, v.y);
		v.y *= -1.f;

		return v;
	};

	constexpr unsigned colors[4] = {
		0xFF0000FF, // red, north
		0x00FF00FF, // green, east
		0x0000FFFF, // blue, south
		0xFFFF00FF // yellow, west
	};

	int side = 0;
	for (size_t i = startNdx; i < startNdx + 24; i += 6) {
		if (const ColliderSurface* s = quad->getSurface((Cardinal)side)) {

			Vec2f n = normal(s->surface);
			Color c = colors[side];
			
			array[i].color = c;
			array[i].pos = s->surface.p1;

			array[i + 1].color = c;
			array[i + 1].pos = s->surface.p2;

			array[i + 2].color = c;
			array[i + 2].pos = s->surface.p1 - n;

			array[i + 3].color = c;
			array[i + 3].pos = s->surface.p2 - n;

			array[i + 4].color = c;
			array[i + 4].pos = s->surface.p1 - n;

			array[i + 5].color = c;
			array[i + 5].pos = s->surface.p2;
		}
		side++;
	}
}


bool debugDrawQuad(ColliderQuad& quad, Vec2f offset, const void* sign, bool always_redraw) {
	
	if (!debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_COLLIDER))
		return false;

	if (!always_redraw && sign && debug_draw::repeat(sign, offset)) {
		return false;
	}

	debug_draw::set_offset(offset);

	auto& draw = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_COLLIDER>(sign, Primitive::TRIANGLES, 24);

	debug_impl(&quad, draw, 0);

	debug_draw::set_offset();
	return true;
}

bool debugDrawQuad(size_t count, ColliderQuad* quad, Vec2f offset, const void* sign, bool always_redraw) {
		

	if (!debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_COLLIDER))
		return false;

	if (!always_redraw && sign && debug_draw::repeat(sign, offset)) {
		return false;
	}

	debug_draw::set_offset(offset);

	auto& draw = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_COLLIDER>(sign, Primitive::TRIANGLES, count * 24);

	size_t ndx = 0;
	for (size_t i = 0; i < count; ndx++) {
		if (!(quad + ndx)->hasAnySurface()) {
			continue;
		}

		debug_impl(quad + ndx, draw, i * 24);
		i++;
	}
	debug_draw::set_offset();
	return true;
}

// -------------------------------------------


ColliderQuad::ColliderQuad() noexcept :
	surfaces{}
{
}


ColliderQuad::ColliderQuad(const cardinal_array<QuadSurface>& surfaces) noexcept :
	surfaces{ surfaces }
{
}

ColliderQuad::ColliderQuad(cardinal_array<QuadSurface>&& surfaces) noexcept :
	surfaces{ surfaces }
{
}

ColliderQuad::ColliderQuad(const Rectf& shape) noexcept {

	ordinal_array<Vec2f> points = {
		math::rect_topleft(shape),
		math::rect_topright(shape),
		math::rect_botright(shape),
		math::rect_botleft(shape)
	};

	QuadSurface* prev = nullptr;
	for (auto dir : direction::cardinals)
	{
		surfaces[dir].hasSurface = true;

		surfaces[dir].collider.g0virtual = false;
		surfaces[dir].collider.g3virtual = false;

		auto [ord1, ord2] = direction::split(dir);

		surfaces[dir].collider.ghostp0		= points[direction::opposite(ord2)];
		surfaces[dir].collider.surface.p1	= points[ord1];
		surfaces[dir].collider.surface.p2	= points[ord2];
		surfaces[dir].collider.ghostp3		= points[direction::opposite(ord1)];

		if (prev) {
			surfaces[dir].collider.prev_id = prev->collider.id;
			prev->collider.next_id = surfaces[dir].collider.id;
		}
		prev = &surfaces[dir];
	}
	if (prev) {
		surfaces[Cardinal::N].collider.prev_id = prev->collider.id;
		prev->collider.next_id = surfaces[Cardinal::N].collider.id;
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
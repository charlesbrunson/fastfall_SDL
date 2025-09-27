#include "fastfall/game/phys/collider_coretypes/ColliderQuad.hpp"

#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/render/drawable/VertexArray.hpp"

#include "fastfall/util/log.hpp"

namespace ff {


constexpr void debug_impl(ColliderQuad* quad, std::span<Vertex> array, size_t startNdx) {
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
	
	if (!debug::enabled(debug::Collision_Collider))
		return false;

	if (!always_redraw && sign && debug::repeat(sign, offset)) {
		return false;
	}

	auto draw = debug::draw(sign, Primitive::TRIANGLES, 24, offset);
	debug_impl(&quad, draw, 0);

	return true;
}

bool debugDrawQuad(size_t count, ColliderQuad* quad, Vec2f offset, const void* sign, bool always_redraw) {
		

	if (!debug::enabled(debug::Collision_Collider))
		return false;

	if (!always_redraw && sign && debug::repeat(sign, offset)) {
		return false;
	}


	auto draw = debug::draw(sign, Primitive::TRIANGLES, count * 24, offset);

	size_t ndx = 0;
	for (size_t i = 0; i < count; ndx++) {
		if (!(quad + ndx)->hasAnySurface()) {
			continue;
		}

		debug_impl(quad + ndx, draw, i * 24);
		i++;
	}
	return true;
}

// -------------------------------------------


ColliderQuad::ColliderQuad() noexcept :
	surfaces{}
{
	for (auto dir : direction::cardinals) {
		surfaces[dir].collider.id.dir = dir;
	}
}


ColliderQuad::ColliderQuad(const cardinal_array<QuadSurface>& other_surfaces) noexcept :
	surfaces{ other_surfaces }
{
}

ColliderQuad::ColliderQuad(cardinal_array<QuadSurface>&& other_surfaces) noexcept :
	surfaces{ other_surfaces }
{
}

ColliderQuad::ColliderQuad(const Rectf& shape) noexcept 
{
	for (auto dir : direction::cardinals) {
		surfaces[dir].collider.id.dir = dir;
	}

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

ColliderSurface findColliderGhosts(const std::vector<const ColliderQuad*>& nearby, const ColliderSurface& surface) {
    ColliderSurface copy = surface;
    std::vector<const ColliderSurface*> candidatesg0;
    std::vector<const ColliderSurface*> candidatesg3;

    for (auto tile : nearby) {
        if (!tile)
            continue;

        for (auto& qsurf : tile->surfaces) {
            if (!qsurf.hasSurface)
                continue;

            if (qsurf.collider.surface.p2 == surface.surface.p1
                && qsurf.collider.surface.reverse() != surface.surface)
            {
                candidatesg0.push_back(&qsurf.collider);
            }
            else if (qsurf.collider.surface.p1 == surface.surface.p2
                 && qsurf.collider.surface.reverse() != surface.surface)
             {
                candidatesg3.push_back(&qsurf.collider);
            }
        }
    }

    auto v = math::vector(surface.surface);
    auto vvert = math::is_vertical(v);
    Angle idealAng(atan2f(v.y, v.x));

    // select ghosts based on the angle diff closest to zero
    auto g0 = std::min_element(candidatesg0.begin(), candidatesg0.end(),
        [&](const ColliderSurface* lhs, const ColliderSurface* rhs) -> bool {
            if (lhs == rhs)
                return false;

            auto v1 = math::vector(Linef(lhs->surface.p1, surface.surface.p1));
            auto v2 = math::vector(Linef(rhs->surface.p1, surface.surface.p1));

            auto v1vert = math::is_vertical(v1);
            auto v2vert = math::is_vertical(v2);

            // if comparing floor surface to ceil surface,
            // favor the one that v also is floor or ceil
            if (!v1vert && !v2vert && !vvert
                && ((v1.x < 0.f) != (v2.x < 0.f)))
            {
                return (v1.x < 0.f) == (v.x < 0.f);
            }

            // stay non vertical
            if (!vvert && (v1vert != v2vert))
            {
                return !v1vert;
            }

            // otherwise compare by angle
            Angle lhsAng(atan2f(v1.y, v1.x));
            Angle rhsAng(atan2f(v2.y, v2.x));

            float lhsDiff = std::abs((lhsAng - idealAng).radians());
            float rhsDiff = std::abs((rhsAng - idealAng).radians());
            return lhsDiff < rhsDiff;
        });

    auto g3 = std::min_element(candidatesg3.begin(), candidatesg3.end(),
        [&](const ColliderSurface* lhs, const ColliderSurface* rhs) -> bool {
            if (lhs == rhs)
                return false;

            auto v1 = math::vector(Linef(surface.surface.p2, lhs->surface.p2));
            auto v2 = math::vector(Linef(surface.surface.p2, rhs->surface.p2));

            auto v1vert = math::is_vertical(v1);
            auto v2vert = math::is_vertical(v2);

            // if comparing floor surface to ceil surface,
            // favor the one that v also is floor or ceil
            if (!v1vert && !v2vert && !vvert
                && ((v1.x < 0.f) != (v2.x < 0.f)))
            {
                return (v1.x < 0.f) == (v.x < 0.f);
            }

            // stay non vertical
            if (!vvert && (v1vert != v2vert))
            {
                return !v1vert;
            }

            // otherwise compare by angle
            Angle lhsAng(atan2f(v1.y, v1.x));
            Angle rhsAng(atan2f(v2.y, v2.x));

            float lhsDiff = std::abs((lhsAng - idealAng).radians());
            float rhsDiff = std::abs((rhsAng - idealAng).radians());
            return lhsDiff < rhsDiff;
        });

    copy.g0virtual = (g0 == candidatesg0.end());
    if (copy.g0virtual) {
        copy.prev_id = std::nullopt;
        copy.ghostp0 = surface.surface.p1 - v;
    }
    else {
        copy.prev_id = (*g0)->id;
        copy.ghostp0 = (*g0)->surface.p1;
    }

    copy.g3virtual = (g3 == candidatesg3.end());
    if (copy.g3virtual) {
        copy.next_id = std::nullopt;
        copy.ghostp3 = surface.surface.p2 + v;
    }
    else {
        copy.next_id = (*g3)->id;
        copy.ghostp3 = (*g3)->surface.p2;
    }
    return copy;
}

}
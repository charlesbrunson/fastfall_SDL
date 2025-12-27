#pragma once

#include "fastfall/game/phys/collider_coretypes/ColliderQuadID.hpp"
#include "fastfall/game/phys/collider_coretypes/ColliderSurface.hpp"
#include "fastfall/game/tile/Tile.hpp"

#include <cassert>

namespace ff {

// base class of a convex shape composed of at most four ColliderSurfaces
class ColliderQuad {
public:
	constexpr static unsigned MAX_SURFACES = 4;

	struct QuadSurface {
		constexpr QuadSurface() noexcept : collider(ColliderSurface{})
		{
		}

		constexpr explicit QuadSurface(ColliderSurface colliderSurface, bool surfaceExists = true) noexcept :
			hasSurface(surfaceExists),
			collider(colliderSurface)
		{
		}

		friend bool operator==(const QuadSurface& lhs, const QuadSurface& rhs) {
			return lhs.hasSurface == rhs.hasSurface
				&& lhs.collider.surface == rhs.collider.surface;
		}

		bool hasSurface = false;
		ColliderSurface collider;
		SurfaceMaterial material;
	};

	ColliderQuad() noexcept;
	explicit ColliderQuad(const cardinal_array<QuadSurface>& surfaces) noexcept;
	explicit ColliderQuad(cardinal_array<QuadSurface>&& surfaces) noexcept;
	explicit ColliderQuad(const Rectf& shape) noexcept;

	void translate(Vec2f offset) noexcept;


	[[nodiscard]] const ColliderSurface* getSurface(Cardinal side) const noexcept;
	ColliderSurface* getSurface(Cardinal side) noexcept;
	void setSurface(Cardinal side, const ColliderSurface& surface) noexcept;
	void removeSurface(Cardinal side);

	void clearSurfaces() noexcept {
		surfaces[Cardinal::N].hasSurface = false;
		surfaces[Cardinal::E].hasSurface = false;
		surfaces[Cardinal::S].hasSurface = false;
		surfaces[Cardinal::W].hasSurface = false;
	}

	[[nodiscard]] bool hasAnySurface() const noexcept {
		return surfaces[Cardinal::N].hasSurface
			|| surfaces[Cardinal::E].hasSurface
			|| surfaces[Cardinal::S].hasSurface
			|| surfaces[Cardinal::W].hasSurface;
	};

	[[nodiscard]] bool isOneWay(Cardinal dir) const {
		return hasOneWay && oneWayDir == dir;
	}
	[[nodiscard]] bool isBoundary(Cardinal dir) const {
		return hasBoundary && oneWayDir == dir;
	}

	[[nodiscard]] QuadID getID() const noexcept { return quad_id; };
	void setID(QuadID id) {
		assert(id.value >= 0);
		quad_id = id;

		for (auto& surf : surfaces)	{
			surf.collider.id.quad_id = quad_id;
		}
	};

	friend bool operator==(const ColliderQuad& lhs, const ColliderQuad& rhs) {
		return lhs.surfaces[Cardinal::N] == rhs.surfaces[Cardinal::N]
			&& lhs.surfaces[Cardinal::E] == rhs.surfaces[Cardinal::E]
			&& lhs.surfaces[Cardinal::S] == rhs.surfaces[Cardinal::S]
			&& lhs.surfaces[Cardinal::W] == rhs.surfaces[Cardinal::W];
	}

    [[nodiscard]] std::optional<Rectf> get_bounds() const {
        std::optional<Rectf> bounds{};
        for (auto& surf : surfaces) {
            if (!bounds) {
                bounds = math::line_bounds(surf.collider.surface);
            }
            else {
                *bounds = math::rect_bounds(*bounds, math::line_bounds(surf.collider.surface));
            }
        }
        return bounds;
    }

	bool hasOneWay = false;
	bool hasBoundary = false;
	Cardinal oneWayDir = Cardinal::N;
	cardinal_array<QuadSurface> surfaces;

	const TileMaterial* material = nullptr;
	Cardinal matFacing = Cardinal::N;

protected:
	QuadID quad_id;
};

ColliderSurface findColliderGhosts(const std::vector<const ColliderQuad*>& nearby, const ColliderSurface& surface);

bool debugDrawQuad(ColliderQuad& quad, Vec2f offset = Vec2f{}, const void* sign = nullptr, bool always_redraw = false);
bool debugDrawQuad(size_t count, ColliderQuad* quad, Vec2f offset = Vec2f{}, const void* sign = nullptr, bool always_redraw = false);

}
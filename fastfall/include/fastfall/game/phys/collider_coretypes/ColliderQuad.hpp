#pragma once

#include "ColliderSurface.hpp"
#include "fastfall/game/level/Tile.hpp"

#include <array>
#include <assert.h>

namespace ff {

// base class of a convex shape composed of at most four ColliderSurfaces
class ColliderQuad {
public:
	constexpr static unsigned MAX_SURFACES = 4;

	struct QuadSurface {
		QuadSurface() noexcept :
			hasSurface(false),
			collider(ColliderSurface{}),
			material(SurfaceMaterial{})
		{

		}
		constexpr QuadSurface(ColliderSurface colliderSurface, bool surfaceExists = true) noexcept :
			hasSurface(surfaceExists),
			collider(colliderSurface)
		{

		}
		friend inline bool operator==(const QuadSurface& lhs, const QuadSurface& rhs) {
			return lhs.hasSurface == rhs.hasSurface
				&& lhs.collider.surface == rhs.collider.surface;
		}



		bool hasSurface = false;
		ColliderSurface collider;
		SurfaceMaterial material;
	};
	//typedef std::array<QuadSurface, 4> SurfaceArray;


	ColliderQuad() noexcept;
	ColliderQuad(const cardinal_array<QuadSurface>& surfaces) noexcept;
	ColliderQuad(cardinal_array<QuadSurface>&& surfaces) noexcept;
	ColliderQuad(const Rectf& shape) noexcept;

	void translate(Vec2f offset) noexcept;


	const ColliderSurface* getSurface(Cardinal side) const noexcept;
	ColliderSurface* getSurface(Cardinal side) noexcept;
	void setSurface(Cardinal side, const ColliderSurface& surface) noexcept;

	void removeSurface(Cardinal side);

	inline void clearSurfaces() noexcept {

		for (auto dir : direction::cardinals)
		{
			surfaces[dir].hasSurface = false;
		}
	}

	inline bool hasAnySurface() const noexcept {
		return surfaces[Cardinal::N].hasSurface
			|| surfaces[Cardinal::E].hasSurface
			|| surfaces[Cardinal::S].hasSurface
			|| surfaces[Cardinal::W].hasSurface;
	};

	inline bool isOneWay(Cardinal dir) const {
		return hasOneWay && oneWayDir == dir;
	}
	inline bool isBoundary(Cardinal dir) const {
		return hasBoundary && oneWayDir == dir;
	}

	inline int getID() const noexcept { return quad_id; };
	inline void setID(int id) {
		assert(id >= 0);
		quad_id = id;
	};

	friend inline bool operator==(const ColliderQuad& lhs, const ColliderQuad& rhs) {
		return lhs.surfaces[Cardinal::N] == rhs.surfaces[Cardinal::N]
			&& lhs.surfaces[Cardinal::E] == rhs.surfaces[Cardinal::E]
			&& lhs.surfaces[Cardinal::S] == rhs.surfaces[Cardinal::S]
			&& lhs.surfaces[Cardinal::W] == rhs.surfaces[Cardinal::W];
	}

	bool hasOneWay = false;
	bool hasBoundary = false;
	Cardinal oneWayDir = Cardinal::N;
	cardinal_array<QuadSurface> surfaces;

	const TileMaterial* material = nullptr;
	Cardinal matFacing = Cardinal::N;

protected:
	int quad_id = -1;

};

bool debugDrawQuad(ColliderQuad& quad, Vec2f offset = Vec2f{}, const void* sign = nullptr, bool always_redraw = false);
bool debugDrawQuad(size_t count, ColliderQuad* quad, Vec2f offset = Vec2f{}, const void* sign = nullptr, bool always_redraw = false);

}
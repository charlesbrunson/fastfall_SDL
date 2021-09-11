#pragma once

#include "ColliderSurface.hpp"
#include "fastfall/util/cardinal.hpp"
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
	typedef std::array<QuadSurface, 4> SurfaceArray;


	ColliderQuad() noexcept;
	ColliderQuad(const SurfaceArray& surfaces) noexcept;
	ColliderQuad(SurfaceArray&& surfaces) noexcept;
	ColliderQuad(const Rectf& shape) noexcept;

	void translate(Vec2f offset) noexcept;


	const ColliderSurface* getSurface(Cardinal side) const noexcept;
	ColliderSurface* getSurface(Cardinal side) noexcept;
	void setSurface(Cardinal side, const ColliderSurface& surface) noexcept;

	void removeSurface(Cardinal side);

	inline void clearSurfaces() noexcept {
		surfaces[0].hasSurface = false;
		surfaces[1].hasSurface = false;
		surfaces[2].hasSurface = false;
		surfaces[3].hasSurface = false;
	}

	inline bool hasAnySurface() const noexcept {
		return
			surfaces[0].hasSurface ||
			surfaces[1].hasSurface ||
			surfaces[2].hasSurface ||
			surfaces[3].hasSurface;
	};
	/*
	inline const SurfaceArray& getSurfaces() const noexcept { return surfaces; };
	*/
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
		/*
		return lhs.hasOneWay == rhs.hasOneWay
			&& lhs.hasBoundary == rhs.hasBoundary
			&& lhs.oneWayDir == rhs.oneWayDir
			&& lhs.surfaces[0] == rhs.surfaces[0]
			&& lhs.surfaces[1] == rhs.surfaces[1]
			&& lhs.surfaces[2] == rhs.surfaces[2]
			&& lhs.surfaces[3] == rhs.surfaces[3]
			&& lhs.material == rhs.material
			&& lhs.matFacing == rhs.matFacing;
		*/
		return lhs.surfaces[0] == rhs.surfaces[0]
			&& lhs.surfaces[1] == rhs.surfaces[1]
			&& lhs.surfaces[2] == rhs.surfaces[2]
			&& lhs.surfaces[3] == rhs.surfaces[3];
	}

	//protected:

	bool hasOneWay = false;
	bool hasBoundary = false;
	Cardinal oneWayDir = CARDINAL_FIRST;
	SurfaceArray surfaces;

	const TileMaterial* material = nullptr;
	Cardinal matFacing = Cardinal::NORTH;

protected:
	int quad_id = -1;

};

void debugDrawQuad(ColliderQuad& quad, Vec2f offset = Vec2f{}, const void* sign = nullptr, bool always_redraw = false);
void debugDrawQuad(size_t count, ColliderQuad* quad, Vec2f offset = Vec2f{}, const void* sign = nullptr, bool always_redraw = false);

}
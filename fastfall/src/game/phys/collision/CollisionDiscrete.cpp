#include "fastfall/game/phys/collision/CollisionDiscrete.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/engine/config.hpp"

namespace ff {

	CollisionDiscrete::CollisionDiscrete(const Collidable* collidable, const ColliderQuad* collisionTile, const ColliderRegion* colliderRegion, bool collidePreviousFrame) :
		cAble(collidable),
		cTile(collisionTile),
		cQuad(*collisionTile),
		collidePrevious(collidePreviousFrame),
		region(colliderRegion)
{
	//axes.reserve(8u);
	reset(collisionTile, colliderRegion, collidePreviousFrame);
}

void CollisionDiscrete::reset(const ColliderQuad* collisionTile, const ColliderRegion* colliderRegion, bool collidePreviousFrame) {
	collidePrevious = collidePreviousFrame;
	cQuad = *collisionTile;

	valley_NE = false;
	valley_SE = false;

	valley_NW = false;
	valley_SW = false;

	//evaluated = false;

	contact = Contact{};

	Vec2f topLeft(FLT_MAX, FLT_MAX);
	Vec2f botRight(-FLT_MAX, -FLT_MAX);

	cQuad.translate(collidePrevious ? colliderRegion->getPrevPosition() : colliderRegion->getPosition());

	for (auto& surface : cQuad.surfaces) {
		topLeft.x = std::min(surface.collider.surface.p1.x, topLeft.x);
		topLeft.x = std::min(surface.collider.surface.p2.x, topLeft.x);

		topLeft.y = std::min(surface.collider.surface.p1.y, topLeft.y);
		topLeft.y = std::min(surface.collider.surface.p2.y, topLeft.y);

		botRight.x = std::max(surface.collider.surface.p1.x, botRight.x);
		botRight.x = std::max(surface.collider.surface.p2.x, botRight.x);

		botRight.y = std::max(surface.collider.surface.p1.y, botRight.y);
		botRight.y = std::max(surface.collider.surface.p2.y, botRight.y);
	}
	assert(topLeft.x <= botRight.x && topLeft.y <= botRight.y);

	tArea = Rectf(topLeft, botRight - topLeft);

	if (tArea.height == 0.f) {

		if (cQuad.isOneWay(Cardinal::NORTH) || cQuad.isBoundary(Cardinal::NORTH)) {
			tArea.height = TILESIZE_F;
		}
		else if (cQuad.isOneWay(Cardinal::SOUTH) || cQuad.isBoundary(Cardinal::SOUTH)) {
			tArea.top -= TILESIZE_F;
			tArea.height = TILESIZE_F;
		}

	}
	else if (tArea.width == 0.f) {

		if (cQuad.isOneWay(Cardinal::WEST) || cQuad.isBoundary(Cardinal::WEST)) {
			tArea.width = TILESIZE_F;
		}
		else if (cQuad.isOneWay(Cardinal::EAST) || cQuad.isBoundary(Cardinal::EAST)) {
			tArea.left -= TILESIZE_F;
			tArea.width = TILESIZE_F;
		}
	}

	assert(tArea.width > 0.f);
	assert(tArea.height > 0.f);

	tPos = math::rect_topleft(tArea);
	tMid = math::rect_mid(tArea);
	tHalf = tArea.getSize() / 2.f;

	initCollidableData();

	createAxes();
	updateContact();
	evalContact();
}

void CollisionDiscrete::createAxes() noexcept {
	//axes.clear();

	axis_count = 0;
	bool hasFloor = false;
	bool hasCeil = false;
	bool hasEast = false;
	bool hasWest = false;
	bool hasEastCorner = false;
	bool hasWestCorner = false;

	static constexpr uint8_t PRESTEP_MAX = 4u;
	std::array<AxisPreStep, PRESTEP_MAX> verticals;
	std::array<AxisPreStep, PRESTEP_MAX> non_verticals;
	uint8_t vSize = 0u;
	uint8_t hSize = 0u;

	for (uint8_t ndx = 0u; ndx < cQuad.surfaces.size(); ndx++) {

		const auto& surface = cQuad.surfaces.at(ndx);

		if (!surface.hasSurface)
			continue;

		const ColliderSurface& surf = surface.collider;
		Vec2f v = math::vector(surface.collider.surface);

		assert(v != Vec2f());

		if (v.x == 0.f) {
			if (v.y > 0.f) {
				hasEast = true;
				verticals[vSize++] = AxisPreStep{ .dir = Cardinal::EAST, .surface = surf, .is_real = true, .is_valid = true, .quadNdx = ndx };
			}
			else {
				hasWest = true;
				verticals[vSize++] = AxisPreStep{ .dir = Cardinal::WEST, .surface = surf, .is_real = true, .is_valid = true, .quadNdx = ndx };
			}
		}
		else if (v.x > 0.f) {
			hasFloor = true;
			non_verticals[hSize++] = AxisPreStep{ .dir = Cardinal::NORTH, .surface = surf, .is_real = true, .is_valid = true, .quadNdx = ndx };

			if (surface.hasSurface && !math::is_horizontal(v)) {
				if (surf.ghostp3.x <= surf.surface.p2.x && surf.ghostp3.y >= surf.surface.p2.y /*&& surf.surface.p2.y > surf.surface.p1.y*/) {
					//east corner
					if (!hasEastCorner) {
						hasEastCorner = true;
						ColliderSurface corner;
						corner.ghostp0 = surf.surface.p1;
						corner.surface.p1 = surf.surface.p2;
						corner.surface.p2 = surf.surface.p2;
						corner.ghostp3 = surf.ghostp3;

						verticals[vSize++] = AxisPreStep{ .dir = Cardinal::EAST, .surface = corner, .is_real = false, .is_valid = true, .quadNdx = ndx };
					}

				}
				if (surf.ghostp0.x >= surf.surface.p1.x && surf.ghostp0.y >= surf.surface.p1.y /*&& surf.surface.p1.y > surf.surface.p2.y*/) {
					//west corner
					if (!hasWestCorner) {
						hasWestCorner = true;
						ColliderSurface corner;
						corner.ghostp0 = surf.ghostp0;
						corner.surface.p1 = surf.surface.p1;
						corner.surface.p2 = surf.surface.p1;
						corner.ghostp3 = surf.surface.p2;

						verticals[vSize++] = AxisPreStep{ .dir = Cardinal::WEST, .surface = corner, .is_real = false, .is_valid = true, .quadNdx = ndx };
					}
				}

				// east peak
				/*
				if (math::is_vertical(Linef(surf.surface.p2, surf.ghostp3)) && surf.surface.p2.y < surf.ghostp3.y) {
					LOG_INFO("EAST N PEAK");
				}
				// west peak
				else if (math::is_vertical(Linef(surf.ghostp0, surf.surface.p1)) && surf.surface.p1.y < surf.ghostp0.y) {
					LOG_INFO("WEST N PEAK");
				}
				*/
			}
		}
		else {
			hasCeil = true;
			non_verticals[hSize++] = AxisPreStep{ .dir = Cardinal::SOUTH, .surface = surf, .is_real = true, .is_valid = true, .quadNdx = ndx };

			if (surface.hasSurface && !math::is_horizontal(v)) {
				if (surf.ghostp0.x <= surf.surface.p1.x && surf.ghostp0.y <= surf.surface.p1.y /*&& surf.surface.p1.y < surf.surface.p2.y*/) {
					//east corner
					if (!hasEastCorner) {
						hasEastCorner = true;
						ColliderSurface corner;
						corner.ghostp0 = surf.ghostp0;
						corner.surface.p1 = surf.surface.p1;
						corner.surface.p2 = surf.surface.p1;
						corner.ghostp3 = surf.surface.p2;

						verticals[vSize++] = AxisPreStep{ .dir = Cardinal::EAST, .surface = corner, .is_real = false, .is_valid = true, .quadNdx = ndx };
					}
				}
				if (surf.ghostp3.x >= surf.surface.p2.x && surf.ghostp3.y <= surf.surface.p2.y /*&& surf.surface.p2.y < surf.surface.p1.y*/) {
					//west corner
					if (!hasWestCorner) {
						hasWestCorner = true;
						ColliderSurface corner;
						corner.ghostp0 = surf.surface.p1;
						corner.surface.p1 = surf.surface.p2;
						corner.surface.p2 = surf.surface.p2;
						corner.ghostp3 = surf.ghostp3;

						verticals[vSize++] = AxisPreStep{ .dir = Cardinal::WEST, .surface = corner, .is_real = false, .is_valid = true, .quadNdx = ndx };
					}
				}

				// west peak
				/*
				if (math::is_vertical(Linef(surf.surface.p2, surf.ghostp3)) && surf.surface.p2.y > surf.ghostp3.y) {
					LOG_INFO("WEST S PEAK");
				}
				// east peak
				else if (math::is_vertical(Linef(surf.ghostp0, surf.surface.p1)) && surf.surface.p1.y > surf.ghostp0.y) {
					LOG_INFO("EAST S PEAK");
				}
				*/
			}
		}
	}

	std::sort(verticals.begin(), verticals.begin() + vSize,
		[](const AxisPreStep& lhs, const AxisPreStep& rhs) {
			return lhs.dir < rhs.dir;
		});

	std::sort(non_verticals.begin(), non_verticals.begin() + hSize,
		[](const AxisPreStep& lhs, const AxisPreStep& rhs) {
			return lhs.dir < rhs.dir;
		});


	// generate remaining axes with bounding box
	// these axis will only be used to determine intersection
	// and cannot be selected for the resolving axis

	if (!hasFloor) {
		ColliderSurface fake;
		fake.surface.p1 = math::rect_topleft(tArea);
		fake.surface.p2 = math::rect_topright(tArea);
		non_verticals[hSize++] = AxisPreStep{ .dir = Cardinal::NORTH, .surface = fake, .is_real = false, .is_valid = false, .quadNdx = 255u };
	}
	if (!hasCeil) {
		ColliderSurface fake;
		fake.surface.p1 = math::rect_botright(tArea);
		fake.surface.p2 = math::rect_botleft(tArea);
		non_verticals[hSize++] = AxisPreStep{ .dir = Cardinal::SOUTH, .surface = fake, .is_real = false, .is_valid = false, .quadNdx = 255u };
	}
	if (!hasEast && !hasEastCorner) {
		ColliderSurface fake;
		fake.surface.p1 = math::rect_topright(tArea);
		fake.surface.p2 = math::rect_botright(tArea);
		verticals[vSize++] = AxisPreStep{ .dir = Cardinal::EAST, .surface = fake, .is_real = false,	.is_valid = false, .quadNdx = 255u };
	}
	if (!hasWest && !hasWestCorner) {
		ColliderSurface fake;
		fake.surface.p1 = math::rect_botleft(tArea);
		fake.surface.p2 = math::rect_topleft(tArea);
		verticals[vSize++] = AxisPreStep{ .dir = Cardinal::WEST, .surface = fake, .is_real = false, .is_valid = false, .quadNdx = 255u };
	}
	//LOG_INFO("{} {}", hSize, vSize);

	for (size_t i = 0u; i < hSize; i++) {
		switch (non_verticals[i].dir) {
		case Cardinal::NORTH: axes[axis_count++] = createFloor(non_verticals[i]); break;
		case Cardinal::SOUTH: axes[axis_count++] = createCeil(non_verticals[i]); break;

		// dont handle EAST or WEST
		default: break;
		}
	}
	for (size_t i = 0u; i < vSize; i++) {
		switch (verticals[i].dir) {
		case Cardinal::EAST:
			if ((hasEast && hasEastCorner && verticals[i].is_real) || (!hasEast || !hasEastCorner))
				axes[axis_count++] = createEastWall(verticals[i]);
			break;
		case Cardinal::WEST:
			if ((hasWest && hasWestCorner && verticals[i].is_real) || (!hasWest || !hasWestCorner))
				axes[axis_count++] = createWestWall(verticals[i]);
			break;
		// dont handle NORTH or SOUTH
		default: break;
		}
	}
}

void CollisionDiscrete::updateContact() noexcept {

	// assume collidable has changed position/size
	initCollidableData();

	// calculate separation, position, collider_normal
	//for (auto& axis : axes) {
	for (unsigned i = 0; i < axis_count; i++)
	{
		auto& axis = axes[i];

		if (axis.dir == Cardinal::NORTH) {

			float Y = tArea.top;
			if (!math::is_horizontal(axis.contact.collider.surface)) {
				Y = getYforX(axis.contact.collider.surface, cMid.x);

				if (cPrev.top + cPrev.height <= tArea.top &&
					Y <= tArea.top &&
					(axis.quadIndex != 255U) &&
					cQuad.getSurface(Cardinal(axis.quadIndex)) &&
					math::clamp(cMid.x, tArea.left, tArea.left + tArea.width) != cMid.x)
				{
					axis.contact.collider_normal = Vec2f(0.f, -1.f);
					Y = tArea.top;
				}
				else {
					axis.contact.collider_normal = math::vector(axis.contact.collider.surface).lefthand().unit();
				}
			}


			axis.contact.separation = -Y + (cMid.y + cHalf.y);
			axis.contact.position = Vec2f(math::clamp(cMid.x, tArea.left, tArea.left + tArea.width), Y);

		}
		else if (axis.dir == Cardinal::SOUTH) {

			float Y = tArea.top + tArea.height;
			if (!math::is_horizontal(axis.contact.collider.surface)) {
				Y = getYforX(axis.contact.collider.surface, cMid.x);

				if (cPrev.top >= tArea.top + tArea.height &&
					Y >= tArea.top + tArea.height &&
					(axis.quadIndex != 255U) &&
					cQuad.getSurface(Cardinal(axis.quadIndex)) &&
					math::clamp(cMid.x, tArea.left, tArea.left + tArea.width) != cMid.x)
				{
					axis.contact.collider_normal = Vec2f(0.f, 1.f);
					Y = tArea.top + tArea.height;
				}
				else {
					axis.contact.collider_normal = math::vector(axis.contact.collider.surface).lefthand().unit();
				}
			}

			axis.contact.separation = Y - (cMid.y - cHalf.y);
			axis.contact.position = Vec2f(math::clamp(cMid.x, tArea.left, tArea.left + tArea.width), Y);
		}
		else if (axis.dir == Cardinal::EAST) {

			axis.contact.separation = (tArea.left + tArea.width) - cMid.x + axis.separationOffset;
			axis.contact.position = Vec2f((tArea.left + tArea.width), math::clamp(cMid.y, axis.contact.collider.surface.p1.y, axis.contact.collider.surface.p2.y));

		}
		else if (axis.dir == Cardinal::WEST) {

			axis.contact.separation = cMid.x - (tArea.left) + axis.separationOffset;
			axis.contact.position = Vec2f(tArea.left, math::clamp(cMid.y, axis.contact.collider.surface.p2.y, axis.contact.collider.surface.p1.y));
		}
	}

	evalContact();
}

void CollisionDiscrete::evalContact() noexcept {

	bool hasContact = true;
	unsigned noContactCounter = 0u;
	//for (auto& axis : axes) {
	for (unsigned i = 0; i < axis_count; i++)
	{
		auto& axis = axes[i];


		// some post-processing
		if (axis.dir == Cardinal::NORTH) {
			// follow up on valley check
			if ((valley_NE && cMid.x > math::rect_topright(tArea).x - VALLEY_FLATTEN_THRESH) ||
				(valley_NW && cMid.x < math::rect_topleft(tArea).x + VALLEY_FLATTEN_THRESH)) {
				axis.contact.collider_normal = axis.contact.ortho_normal;

				float nSep = -std::max(axis.contact.collider.surface.p1.y, axis.contact.collider.surface.p2.y) + (cMid.y + cHalf.y);
				axis.contact.separation = std::max(axis.contact.separation, nSep);

			}

			axis.contact.hasValley = valley_NE || valley_NW;
		}
		else if (axis.dir == Cardinal::SOUTH) {
			// follow up on valley check
			if ((valley_SE && cMid.x > math::rect_topright(tArea).x - VALLEY_FLATTEN_THRESH) ||
				(valley_SW && cMid.x < math::rect_topleft(tArea).x + VALLEY_FLATTEN_THRESH)) {
				axis.contact.collider_normal = axis.contact.ortho_normal;

				float nSep = std::min(axis.contact.collider.surface.p1.y, axis.contact.collider.surface.p2.y) - (cMid.y - cHalf.y);


				axis.contact.separation = std::max(axis.contact.separation, nSep);
			}

			axis.contact.hasValley = valley_SE || valley_SW;
		}
		//assert(!std::isnan(axis.contact.separation));
		axis.contact.hasContact = axis.axisValid && axis.is_intersecting();

		if (!axis.contact.hasContact) noContactCounter++;

		hasContact &= axis.contact.hasContact;
	}

	CollisionAxis* bestPick = nullptr;
	CollisionAxis* secondPick = nullptr; // determine best orthogonal movement to touch

	if (noContactCounter == 0u) {
		for (unsigned i = 0; i < axis_count; i++)
		{
			auto& axis = axes[i];

			auto sepCmp = [](auto& axis, auto* best) -> bool {
				return !best || (axis.contact.separation < best->contact.separation);
			};

			if (axis.is_collider_valid() &&	sepCmp(axis, bestPick)) {
				bestPick = &axis;
			}
		}
	}
	else if (noContactCounter == 1u) {
		for (unsigned i = 0; i < axis_count; i++)
		{
			auto& axis = axes[i];
			if (axis.is_collider_valid() &&
				!axis.is_intersecting()) {
				secondPick = &axis;
			}
		}
	}

	if (bestPick && bestPick->contact.separation == FLT_MAX)
		LOG_ERR_("bestPick = FLT_MAX");

	if (!bestPick || bestPick->contact.separation == FLT_MAX) {
		contact = Contact{};
		hasContact = false;
	}

	if (bestPick) {
		contact = bestPick->contact;
	}
	else if (secondPick) {
		contact = secondPick->contact;
		contact.hasContact = false;
	}

	contact.hasContact = hasContact;


}

CollisionAxis CollisionDiscrete::createFloor(const AxisPreStep& initData) noexcept {

	CollisionAxis axis(initData);
	axis.contact.ortho_normal = Vec2f(0.f, -1.f);
	axis.contact.collider_normal = Vec2f(0.f, -1.f);
	if (cQuad.material) {
		axis.contact.material = &cQuad.material->getSurface(Cardinal::NORTH, cQuad.matFacing);
	}
	//axis.contact.

	// if this is a oneway, invalidate it if the collider's previous position is not above it
	if (!collidePrevious && cQuad.isOneWay(Cardinal::NORTH)) {
		axis.axisValid = math::rect_botleft(cPrev).y <= tMid.y - tHalf.y;
	}

	return axis;
}

CollisionAxis CollisionDiscrete::createCeil(const AxisPreStep& initData) noexcept {

	CollisionAxis axis(initData);
	axis.contact.ortho_normal = Vec2f(0.f, 1.f);
	axis.contact.collider_normal = Vec2f(0.f, 1.f);
	if (cQuad.material) {
		axis.contact.material = &cQuad.material->getSurface(Cardinal::SOUTH, cQuad.matFacing);
	}

	// if this is a oneway, invalidate it if the collider's previous position is not below it
	if (!collidePrevious && cQuad.isOneWay(Cardinal::SOUTH)) {
		//LOG_INFO("{} >= {}", math::rect_topleft(cPrev).y, tMid.y + tHalf.y);
		axis.axisValid = math::rect_topleft(cPrev).y >= tMid.y + tHalf.y;
	}

	return axis;
}

CollisionAxis CollisionDiscrete::createEastWall(const AxisPreStep& initData) noexcept {

	CollisionAxis axis(initData);
	axis.contact.ortho_normal = Vec2f(1.f, 0.f);
	axis.contact.collider_normal = Vec2f(1.f, 0.f);
	if (cQuad.material) {
		axis.contact.material = &cQuad.material->getSurface(Cardinal::EAST, cQuad.matFacing);
	}

	bool extend = axis.is_collider_valid();

	// one way edge case
	if (!extend && (cQuad.isOneWay(Cardinal::NORTH) || cQuad.isOneWay(Cardinal::SOUTH))) {
		auto* north_ptr = cQuad.getSurface(Cardinal::NORTH);
		auto* south_ptr = cQuad.getSurface(Cardinal::SOUTH);

		if ((north_ptr && north_ptr->g3virtual) || (south_ptr && south_ptr->g0virtual)) {
			extend = true;
		}
	}

	// check if north or south axis has a valley in it 
	bool has_valley = !axis.is_collider_valid() && std::any_of(std::begin(axes), std::begin(axes) + axis_count,

		[this](CollisionAxis& other_axis) {
			if (other_axis.dir == Cardinal::NORTH && other_axis.is_collider_real()) {
				ColliderSurface& surf = other_axis.contact.collider;

				if (surf.surface.p2.y > surf.ghostp3.y && surf.surface.p2.y > surf.surface.p1.y) {
					valley_NE = true;
					return true;
				}
			}
			else if (other_axis.dir == Cardinal::SOUTH && other_axis.is_collider_real()) {
				ColliderSurface& surf = other_axis.contact.collider;

				if (surf.surface.p1.y < surf.ghostp0.y && surf.surface.p1.y < surf.surface.p2.y) {
					valley_SE = true;
					return true;
				}
			}
			return false;
		});

	// if this is a oneway, invalidate it if the collider's previous position is not left of it
	if (!collidePrevious && cQuad.isOneWay(Cardinal::EAST)) {
		axis.axisValid = math::rect_topleft(cPrev).x >= tPos.x + tArea.width;
	}

	axis.separationOffset = (extend ? cHalf : Vec2f()).x + (has_valley ? VALLEY_FLATTEN_THRESH : 0.f);
	return axis;
}

CollisionAxis CollisionDiscrete::createWestWall(const AxisPreStep& initData) noexcept {

	CollisionAxis axis(initData);
	axis.contact.ortho_normal = Vec2f(-1.f, 0.f);
	axis.contact.collider_normal = Vec2f(-1.f, 0.f);
	if (cQuad.material) {
		axis.contact.material = &cQuad.material->getSurface(Cardinal::WEST, cQuad.matFacing);
	}

	bool extend = axis.is_collider_valid();

	// one way edge case
	if (!extend && (cQuad.isOneWay(Cardinal::NORTH) || cQuad.isOneWay(Cardinal::SOUTH))) {
		auto* north_ptr = cQuad.getSurface(Cardinal::NORTH);
		auto* south_ptr = cQuad.getSurface(Cardinal::SOUTH);

		if ((north_ptr && north_ptr->g0virtual) || (south_ptr && south_ptr->g3virtual)) {
			extend = true;
		}
	}

	// check if north or south axis has a valley in it 
	bool has_valley = !axis.is_collider_valid() && std::any_of(std::begin(axes), std::begin(axes) + axis_count,
		[this](CollisionAxis& other_axis) {
			if (other_axis.dir == Cardinal::NORTH && other_axis.is_collider_real()) {
				ColliderSurface& surf = other_axis.contact.collider;

				if (surf.surface.p1.y > surf.ghostp0.y && surf.surface.p1.y > surf.surface.p2.y) {
					valley_NW = true;
					return true;
				}
			}
			else if (other_axis.dir == Cardinal::SOUTH && other_axis.is_collider_real()) {
				ColliderSurface& surf = other_axis.contact.collider;

				if (surf.surface.p2.y < surf.ghostp3.y && surf.surface.p2.y < surf.surface.p1.y) {
					valley_SW = true;
					return true;
				}
			}
			return false;
		});

	// if this is a oneway, invalidate it if the collider's previous position is not left of it
	if (!collidePrevious && cQuad.isOneWay(Cardinal::WEST)) {
		axis.axisValid = math::rect_topright(cPrev).x <= tArea.left;
	}

	axis.separationOffset = (extend ? cHalf : Vec2f()).x + (has_valley ? VALLEY_FLATTEN_THRESH : 0.f);
	return axis;
}

}
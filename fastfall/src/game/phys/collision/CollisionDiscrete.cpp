#include "fastfall/game/phys/collision/CollisionDiscrete.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/engine/config.hpp"


namespace ff {

inline float getYforX(const Linef& onLine, float X) {
	Vec2f v = math::vector(onLine);
	assert(v.x != 0.f); // no vertical lines
	float scale = ((X - onLine.p1.x) / v.x);
	return (scale * v.y) + onLine.p1.y;
};

CollisionDiscrete::CollisionDiscrete(CollisionContext ctx, ColliderQuad quad, CollisionID t_id, Type collidePrev)
    : id(t_id)
    , collision_time(collidePrev)
{
    reset(ctx, quad, collidePrev);
}

void CollisionDiscrete::reset(CollisionContext ctx, ColliderQuad quad, Type collidePrev) {

	collision_time = collidePrev;
	cQuad = quad;

	valleys = { false, false, false, false };

	contact = {};
    contact.id = id;

    if (collision_time == Type::CurrFrame) {
        collider_deltap = ctx.collider ? ctx.collider->getDeltaPosition() : Vec2f{};
    }
    else {
        collider_deltap = Vec2f{};
    }

	Vec2f topLeft(FLT_MAX, FLT_MAX);
	Vec2f botRight(-FLT_MAX, -FLT_MAX);

	cQuad.translate(collision_time == Type::PrevFrame
        ? ctx.collider->getPrevPosition()
        : ctx.collider->getPosition());

	for (auto& surface : cQuad.surfaces) {
		topLeft.x  = std::min(surface.collider.surface.p1.x, topLeft.x);
		topLeft.x  = std::min(surface.collider.surface.p2.x, topLeft.x);
		topLeft.y  = std::min(surface.collider.surface.p1.y, topLeft.y);
		topLeft.y  = std::min(surface.collider.surface.p2.y, topLeft.y);
		botRight.x = std::max(surface.collider.surface.p1.x, botRight.x);
		botRight.x = std::max(surface.collider.surface.p2.x, botRight.x);
		botRight.y = std::max(surface.collider.surface.p1.y, botRight.y);
		botRight.y = std::max(surface.collider.surface.p2.y, botRight.y);
	}

	tArea = Rectf(topLeft, botRight - topLeft);

	if (tArea.height == 0.f) {

		if (cQuad.isOneWay(Cardinal::N) || cQuad.isBoundary(Cardinal::N)) {
			tArea.height = TILESIZE_F;
		}
		else if (cQuad.isOneWay(Cardinal::S) || cQuad.isBoundary(Cardinal::S)) {
			tArea.top -= TILESIZE_F;
			tArea.height = TILESIZE_F;
		}

	}
	else if (tArea.width == 0.f) {

		if (cQuad.isOneWay(Cardinal::W) || cQuad.isBoundary(Cardinal::W)) {
			tArea.width = TILESIZE_F;
		}
		else if (cQuad.isOneWay(Cardinal::E) || cQuad.isBoundary(Cardinal::E)) {
			tArea.left -= TILESIZE_F;
			tArea.width = TILESIZE_F;
		}
	}

	assert(tArea.width > 0.f);
	assert(tArea.height > 0.f);

	tPos = math::rect_topleft(tArea);
	tMid = math::rect_mid(tArea);
	tHalf = tArea.getSize() / 2.f;

    initCollidableData(ctx);
	createAxes();
	updateContact(ctx);
	evalContact();
}

void CollisionDiscrete::createAxes() noexcept 
{

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

	for (auto dir : direction::cardinals) {
		uint8_t ndx = static_cast<uint8_t>(dir);

		const auto& surface = cQuad.surfaces[dir];

		if (!surface.hasSurface)
			continue;

		const ColliderSurface& surf = surface.collider;
		Vec2f v = math::vector(surface.collider.surface);

		assert(v != Vec2f());

		if (v.x == 0.f) {
			if (v.y > 0.f) {
				hasEast = true;
				verticals[vSize++] = AxisPreStep{ .dir = Cardinal::E, .surface = surf, .is_real = true, .is_valid = true, .quadNdx = ndx };
			}
			else {
				hasWest = true;
				verticals[vSize++] = AxisPreStep{ .dir = Cardinal::W, .surface = surf, .is_real = true, .is_valid = true, .quadNdx = ndx };
			}
		}
		else if (v.x > 0.f) {
			hasFloor = true;
			non_verticals[hSize++] = AxisPreStep{ .dir = Cardinal::N, .surface = surf, .is_real = true, .is_valid = true, .quadNdx = ndx };

			if (surface.hasSurface) {
				if (!hasEastCorner && surf.ghostp3.x <= surf.surface.p2.x && surf.ghostp3.y >= surf.surface.p2.y) {
					//east corner
					hasEastCorner = true;
					verticals[vSize++] = AxisPreStep{ 
						.dir = Cardinal::E, 
						.surface = { {surf.surface.p2, surf.surface.p2}, surf.surface.p1, surf.ghostp3 },
						.is_real = false, 
						.is_valid = true, 
						.quadNdx = ndx 
					};
				}
				if (!hasWestCorner && surf.ghostp0.x >= surf.surface.p1.x && surf.ghostp0.y >= surf.surface.p1.y) {
					//west corner
					hasWestCorner = true;
					verticals[vSize++] = AxisPreStep{ 
						.dir = Cardinal::W, 
						.surface = { {surf.surface.p1, surf.surface.p1}, surf.ghostp0, surf.surface.p2 },
						.is_real = false, 
						.is_valid = true, 
						.quadNdx = ndx 
					};
				}
			}
		}
		else {
			hasCeil = true;
			non_verticals[hSize++] = AxisPreStep{ .dir = Cardinal::S, .surface = surf, .is_real = true, .is_valid = true, .quadNdx = ndx };

			if (surface.hasSurface) {
				if (!hasEastCorner && surf.ghostp0.x <= surf.surface.p1.x && surf.ghostp0.y <= surf.surface.p1.y) {
					//east corner
					hasEastCorner = true;
					verticals[vSize++] = AxisPreStep{ 
						.dir = Cardinal::E, 
						.surface = { {surf.surface.p1, surf.surface.p1}, surf.ghostp0, surf.surface.p2 },
						.is_real = false, 
						.is_valid = true, 
						.quadNdx = ndx 
					};
				}
				if (!hasWestCorner && surf.ghostp3.x >= surf.surface.p2.x && surf.ghostp3.y <= surf.surface.p2.y) {
					//west corner
					hasWestCorner = true;
					verticals[vSize++] = AxisPreStep{ 
						.dir = Cardinal::W, 
						.surface = { {surf.surface.p2, surf.surface.p2}, surf.surface.p1, surf.ghostp3 },
						.is_real = false, 
						.is_valid = true, 
						.quadNdx = ndx 
					};
				}
			}
		}
	}


	// generate remaining axes with bounding box
	// these axis will only be used to determine intersection
	// and cannot be selected for the resolving axis

	auto make_fake = [](Linef line, Cardinal dir) -> AxisPreStep{
		ColliderSurface fake;
		fake.surface = line;
		return AxisPreStep{ .dir = dir, .surface = fake, .is_real = false, .is_valid = false, .quadNdx = 255u };
	};

	auto points = tArea.toPoints();

	if (!hasFloor) {
		non_verticals[hSize++] = make_fake({points[0], points[1]}, Cardinal::N);
	}
	if (!hasCeil) {
		non_verticals[hSize++] = make_fake({points[3], points[2]}, Cardinal::S);
	}
	if (!hasEast && !hasEastCorner) {
		verticals[vSize++] = make_fake({points[1], points[3]}, Cardinal::E);
	}
	if (!hasWest && !hasWestCorner) {
		verticals[vSize++] = make_fake({points[2], points[0]}, Cardinal::W);
	}

	std::sort(verticals.begin(), verticals.begin() + vSize,
		[](const AxisPreStep& lhs, const AxisPreStep& rhs) {
			return lhs.dir < rhs.dir;
		});

	std::sort(non_verticals.begin(), non_verticals.begin() + hSize,
		[](const AxisPreStep& lhs, const AxisPreStep& rhs) {
			return lhs.dir < rhs.dir;
		});

	for (size_t i = 0u; i < hSize; i++) {
		switch (non_verticals[i].dir) {
		case Cardinal::N: axes[axis_count++] = createFloor(non_verticals[i]); break;
		case Cardinal::S: axes[axis_count++] = createCeil(non_verticals[i]); break;
		// dont handle EAST or WEST
		default: break;
		}
	}
	for (size_t i = 0u; i < vSize; i++) {
		switch (verticals[i].dir) {
		case Cardinal::E:
			if ((hasEast && hasEastCorner && verticals[i].is_real) || (!hasEast || !hasEastCorner))
				axes[axis_count++] = createEastWall(verticals[i]);
			break;
		case Cardinal::W:
			if ((hasWest && hasWestCorner && verticals[i].is_real) || (!hasWest || !hasWestCorner))
				axes[axis_count++] = createWestWall(verticals[i]);
			break;
		// dont handle NORTH or SOUTH
		default: break;
		}
	}
}

void CollisionDiscrete::updateContact(CollisionContext ctx) noexcept {

	// assume collidable has changed position/size
	initCollidableData(ctx);

	// calculate separation, position, collider_normal
	for (unsigned i = 0; i < axis_count; i++)
	{
		auto& axis = axes[i];

		if (axis.dir == Cardinal::N) {

			float Y = tArea.top;
			if (!math::is_horizontal(axis.contact.collider.surface)) {
				Y = getYforX(axis.contact.collider.surface, cMid.x);

                float clamp = math::clamp(cMid.x, tArea.left, tArea.left + tArea.width);
                axis.contact.collider_n = math::vector(axis.contact.collider.surface).lefthand().unit();
				if (cPrev.top + cPrev.height <= tArea.top - collider_deltap.y &&
					Y <= tArea.top &&
					(axis.quadIndex != 255U) &&
					cQuad.getSurface(Cardinal(axis.quadIndex)) &&
                    clamp != cMid.x)
				{
                    if (cMid.x > tArea.left + tArea.width
                        && axis.contact.collider.getGhostNext().p2.y >= tArea.top)
                    {
                        axis.contact.collider_n = Vec2f(0.f, -1.f);
                    }
                    else if (cMid.x < tArea.left
                             && axis.contact.collider.getGhostPrev().p1.y >= tArea.top)
                    {
                        axis.contact.collider_n = Vec2f(0.f, -1.f);
                    }
					Y = tArea.top;
				}
			}

			axis.contact.separation = -Y + (cMid.y + cHalf.y);
			axis.contact.position = Vec2f(math::clamp(cMid.x, tArea.left, tArea.left + tArea.width), Y);

			// calc stick
			Vec2f pMid = math::rect_mid(cPrev);
			Linef left{ axis.contact.collider.ghostp0, axis.contact.collider.surface.p1 };
			Linef right{ axis.contact.collider.surface.p2, axis.contact.collider.ghostp3 };

			if (left.p1.x < left.p2.x
				&& cMid.x < tArea.left 
				&& pMid.x >= tArea.left) 
			{
				float stickY = getYforX(left, cMid.x);
				axis.contact.stickOffset = -stickY + (cMid.y + cHalf.y) - axis.contact.separation;
				axis.contact.stickLine = left;
			}
			else if (right.p1.x < right.p2.x
				&& cMid.x > tArea.left + tArea.width
				&& pMid.x <= tArea.left + tArea.width)
            {
				float stickY = getYforX(right, cMid.x);
				axis.contact.stickOffset = -stickY + (cMid.y + cHalf.y) - axis.contact.separation;
				axis.contact.stickLine = right;
			}
		}
		else if (axis.dir == Cardinal::S) {

			float Y = tArea.top + tArea.height;
			if (!math::is_horizontal(axis.contact.collider.surface)) {

				Y = getYforX(axis.contact.collider.surface, cMid.x);
                axis.contact.collider_n = math::vector(axis.contact.collider.surface).lefthand().unit();
                float clamp = math::clamp(cMid.x, tArea.left, tArea.left + tArea.width);

				if (cPrev.top >= tArea.top + tArea.height - collider_deltap.y &&
					Y >= tArea.top + tArea.height &&
					(axis.quadIndex != 255U) &&
					cQuad.getSurface(Cardinal(axis.quadIndex)) &&
                    clamp != cMid.x)
				{
                    if (cMid.x > tArea.left + tArea.width
                        && axis.contact.collider.getGhostPrev().p1.y <= tArea.top)
                    {
                        axis.contact.collider_n = Vec2f(0.f, 1.f);
                    }
                    else if (cMid.x < tArea.left
                         && axis.contact.collider.getGhostNext().p2.y <= tArea.top)
                    {
                        axis.contact.collider_n = Vec2f(0.f, 1.f);
                    }
					Y = tArea.top + tArea.height;
				}
			}

			axis.contact.separation = Y - (cMid.y - cHalf.y);
			axis.contact.position = Vec2f(math::clamp(cMid.x, tArea.left, tArea.left + tArea.width), Y);

			// calc stick
			Vec2f pMid = math::rect_mid(cPrev);
			Linef left{ axis.contact.collider.surface.p2, axis.contact.collider.ghostp3 };
			Linef right{ axis.contact.collider.ghostp0, axis.contact.collider.surface.p1 };

			if (left.p1.x > left.p2.x
				&& cMid.x < tArea.left 
				&& pMid.x >= tArea.left) 
			{
				float stickY = getYforX(left, cMid.x);
				axis.contact.stickOffset = -stickY + (cMid.y - cHalf.y) - axis.contact.separation;
				axis.contact.stickLine = left;
			}
			else if (right.p1.x > right.p2.x
				&& cMid.x > tArea.left + tArea.width
				&& pMid.x <= tArea.left + tArea.width) 
			{
				float stickY = getYforX(right, cMid.x);
				axis.contact.stickOffset = -stickY + (cMid.y - cHalf.y) - axis.contact.separation;
				axis.contact.stickLine = right;
			}
		}
		else if (axis.dir == Cardinal::E) {

			axis.contact.separation = (tArea.left + tArea.width) - cMid.x + axis.separationOffset;
			axis.contact.position = Vec2f((tArea.left + tArea.width), math::clamp(cMid.y, axis.contact.collider.surface.p1.y, axis.contact.collider.surface.p2.y));

		}
		else if (axis.dir == Cardinal::W) {

			axis.contact.separation = cMid.x - (tArea.left) + axis.separationOffset;
			axis.contact.position = Vec2f(tArea.left, math::clamp(cMid.y, axis.contact.collider.surface.p2.y, axis.contact.collider.surface.p1.y));
		}
	}

	evalContact();
}

void CollisionDiscrete::evalContact() noexcept {

	bool hasContact = true;
	unsigned noContactCounter = 0u;
	chosen_axis = -1;

	for (unsigned i = 0; i < axis_count; i++)
	{
		auto& axis = axes[i];

		// some post-processing
		if (axis.dir == Cardinal::N) 
		{
			// follow up on valley check
			if ((valleys[Ordinal::NE] && cMid.x > math::rect_topright(tArea).x - VALLEY_FLATTEN_THRESH) ||
				(valleys[Ordinal::NW] && cMid.x < math::rect_topleft(tArea).x + VALLEY_FLATTEN_THRESH)) 
			{
				axis.contact.collider_n = axis.contact.ortho_n;
				float nSep = -std::max(axis.contact.collider.surface.p1.y, axis.contact.collider.surface.p2.y) + (cMid.y + cHalf.y);
				axis.contact.separation = std::max(axis.contact.separation, nSep);
			}
			axis.contact.hasValley = valleys[Ordinal::NE] || valleys[Ordinal::NW];
		}
		else if (axis.dir == Cardinal::S) 
		{
			// follow up on valley check
			if ((valleys[Ordinal::SE] && cMid.x > math::rect_topright(tArea).x - VALLEY_FLATTEN_THRESH) ||
				(valleys[Ordinal::SW] && cMid.x < math::rect_topleft(tArea).x + VALLEY_FLATTEN_THRESH)) 
			{
				axis.contact.collider_n = axis.contact.ortho_n;
				float nSep = std::min(axis.contact.collider.surface.p1.y, axis.contact.collider.surface.p2.y) - (cMid.y - cHalf.y);
				axis.contact.separation = std::max(axis.contact.separation, nSep);
			}
			axis.contact.hasValley = valleys[Ordinal::SE] || valleys[Ordinal::SW];
		}
		axis.contact.hasContact = axis.axisValid && axis.is_intersecting();

		if (!axis.contact.hasContact) 
			noContactCounter++;

		hasContact &= axis.contact.hasContact;
	}

	CollisionAxis* bestPick = nullptr;
	CollisionAxis* secondPick = nullptr; // determine best orthogonal movement to touch
    CollisionAxis* onewayPick = nullptr;

    for (int i = 0; i < axis_count; i++)
    {
        auto& axis = axes[i];

        if (noContactCounter == 0u) {
            bool is_valid_pick = axis.is_collider_valid();
            bool is_better_pick = !bestPick || (axis.contact.separation < bestPick->contact.separation);
            if (is_valid_pick && is_better_pick) {
                bestPick = &axis;
                chosen_axis = i;
            }
        }
        else if (noContactCounter == 1u) {
            if (axis.is_collider_valid() &&
                !axis.is_intersecting())
            {
                secondPick = &axis;
            }

            if (cQuad.isOneWay(axis.dir)) {
                onewayPick = &axis;
            }
        }
    }

	if (bestPick && bestPick->contact.separation == FLT_MAX)
		LOG_ERR_("bestPick = FLT_MAX");

	if (!bestPick || bestPick->contact.separation == FLT_MAX) {
		contact = {};
		hasContact = false;
	}

	if (bestPick) {
		contact = bestPick->contact;
	}
	else if (secondPick) {
		contact = secondPick->contact;
		chosen_axis = -1;
		//contact.hasContact = false;
	}
    else if (onewayPick) {
        contact = onewayPick->contact;
        chosen_axis = -1;
    }

	contact.hasContact = hasContact;
    contact.id = id;
}

CollisionAxis CollisionDiscrete::createFloor(const AxisPreStep& initData) noexcept {

	CollisionAxis axis(initData);
    axis.contact.id = id;
	axis.contact.ortho_n    = Vec2f(0.f, -1.f);
	axis.contact.collider_n = Vec2f(0.f, -1.f);
	if (cQuad.material) {
		axis.contact.material = &cQuad.material->getSurface(Cardinal::N, cQuad.matFacing);
	}

	// if this is a oneway, invalidate it if the collider's previous position is not above it
	if (collision_time == Type::CurrFrame && cQuad.isOneWay(Cardinal::N))
	{
		Vec2f cpMid = math::rect_mid(cPrev);
		Vec2f cpSize = cPrev.getSize() * 0.5f;

		Linef line;
		bool valid_ghost = true;
		if (cpMid.x < axis.contact.collider.surface.p1.x - collider_deltap.x)
		{
			valid_ghost = !axis.contact.collider.g0virtual;
			line = axis.contact.collider.getGhostPrev();
		}
		else if (cpMid.x > axis.contact.collider.surface.p2.x - collider_deltap.x)
		{
			valid_ghost = !axis.contact.collider.g3virtual;
			line = axis.contact.collider.getGhostNext();
		}
		else 
		{
			line = axis.contact.collider.surface;
		}

        line = math::shift(line, -collider_deltap);
        float slipv = (cSlip.state == Collidable::SlipState::SlipVertical && cVel.y >= 0.f) ? cSlip.leeway : 0.f;
		axis.axisValid = !math::is_vertical(line)
			&& (valid_ghost || slipv != 0)
			&& getYforX(line, cpMid.x) >= cpMid.y + cpSize.y - slipv;
	}

	return axis;
}

CollisionAxis CollisionDiscrete::createCeil(const AxisPreStep& initData) noexcept {

	CollisionAxis axis(initData);
    axis.contact.id = id;
	axis.contact.ortho_n    = Vec2f(0.f, 1.f);
	axis.contact.collider_n = Vec2f(0.f, 1.f);
	if (cQuad.material) {
		axis.contact.material = &cQuad.material->getSurface(Cardinal::S, cQuad.matFacing);
	}

	// if this is a oneway, invalidate it if the collider's previous position is not below it
	if (collision_time == Type::CurrFrame && cQuad.isOneWay(Cardinal::S))
	{
		Vec2f cpMid = math::rect_mid(cPrev);
		Vec2f cpSize = cPrev.getSize() * 0.5f;

		Linef line;
		bool valid_ghost = true;
		if (cpMid.x < axis.contact.collider.surface.p2.x - collider_deltap.x)
		{
			valid_ghost = !axis.contact.collider.g3virtual;
			line = axis.contact.collider.getGhostNext();
		}
		else if (cpMid.x > axis.contact.collider.surface.p1.x - collider_deltap.x)
		{
			valid_ghost = !axis.contact.collider.g0virtual;
			line = axis.contact.collider.getGhostPrev();
		}
		else
		{
			line = axis.contact.collider.surface;
		}
        line = math::shift(line, -collider_deltap);
        float slipv = (cSlip.state == Collidable::SlipState::SlipVertical && cVel.y <= 0.f) ? cSlip.leeway : 0.f;
		axis.axisValid = !math::is_vertical(line)
           && (valid_ghost || slipv != 0)
			&& getYforX(line, cpMid.x) <= cpMid.y - cpSize.y + slipv;
	}

	return axis;
}

// wall utils

bool wallCanExtend(
	const CollisionAxis& axis, 
	const ColliderQuad& quad, 
	Cardinal dir, 
	Vec2f pMid, Vec2f cMid, 
	Vec2f tMid, Vec2f tHalf,
	bool collidePrevious)
{
	if (dir == Cardinal::N
		|| dir == Cardinal::S)
	{
		return false;
	}

	auto* north_ptr = quad.getSurface(Cardinal::N);
	auto* south_ptr = quad.getSurface(Cardinal::S);

	bool extend = axis.is_collider_valid();

	if (!extend && (quad.isOneWay(Cardinal::N) || quad.isOneWay(Cardinal::S))) 
	{
		// extend if the oneway doesn't connects with another floor/ceiling on this side
		if (dir == Cardinal::E)
		{
			extend = ((north_ptr && north_ptr->g3virtual)
				  || (south_ptr && south_ptr->g0virtual));
		}
		else if (dir == Cardinal::W)
		{
			extend = (north_ptr && north_ptr->g0virtual)
				  || (south_ptr && south_ptr->g3virtual);
		}
		else {
			extend = false;
		}
	}
	else if (!collidePrevious && extend)
	{
		// DONT extend if the floor/ceiling continues past the wall
		// but only on the frame that the collision box Y center crosses the wall

		bool prev_above = false;
		bool prev_below = false;

		// are we crossing the X-extent of this side this frame?
		bool passing;
		if (dir == Cardinal::E)
		{
			passing = pMid.x <= tMid.x + tHalf.x
					&& cMid.x > tMid.x + tHalf.x;
		}
		else
		{
            bool check1 = pMid.x >= tMid.x - tHalf.x;
            bool check2 = cMid.x < tMid.x - tHalf.x;

			passing = pMid.x >= tMid.x - tHalf.x
					&& cMid.x < tMid.x - tHalf.x;
		}

		// if prev collision box is above the quad's center
		// check if the north_ptr's ghost extends past the wall
		if (passing && north_ptr)
		{
			Linef line = north_ptr->surface;
			Linef next = (dir == Cardinal::E ? north_ptr->getGhostNext() : north_ptr->getGhostPrev());

			bool still_passing = (dir == Cardinal::E 
				? line.p2.x == tMid.x + tHalf.x 
				: line.p1.x == tMid.x - tHalf.x);

			if (!math::is_vertical(next)
				&& pMid.y < tMid.y
				&& still_passing
				&& (next.p1.x < next.p2.x) == (line.p1.x < line.p2.x))
			{
				prev_above = true;
			}
		}

		// ditto for south_ptr
		if (passing && south_ptr)
		{
			Linef line = south_ptr->surface;
			Linef next = (dir == Cardinal::W ? south_ptr->getGhostNext() : south_ptr->getGhostPrev());

			bool still_passing = (dir == Cardinal::E
				? line.p1.x == tMid.x + tHalf.x
				: line.p2.x == tMid.x - tHalf.x);

			if (!math::is_vertical(next)
				&& pMid.y > tMid.y
                && still_passing
				&& (next.p1.x < next.p2.x) == (line.p1.x < line.p2.x))
			{
				prev_below = true;
			}
		}

		// if either then don't extend
		if (prev_above || prev_below) {
			extend = false;
		}
	}

	return extend;
}

bool wallHasValley(
	const CollisionAxis& curr_axis, 
	const std::array<CollisionAxis, 5>& all_axes, 
	const unsigned axis_count, 
	const Cardinal dir,
	ordinal_array<bool>& valleys_out)
{
	if (   dir == Cardinal::N
		|| dir == Cardinal::S)
	{
		return false;
	}

	bool valid = curr_axis.is_collider_valid();
	bool has_valley = false;

	if (!valid)
	{
		for (unsigned i = 0; i < axis_count; i++) 
		{
			auto& axis = all_axes[i];
			const auto& surf = axis.contact.collider;
			auto ord = direction::combine(axis.dir, dir);

			if (!ord || !axis.is_collider_real())
				continue;

			bool surf_concave = false;
			switch (*ord)
			{
			case Ordinal::NW: 
				surf_concave = surf.surface.p1.y > surf.ghostp0.y 
					        && surf.surface.p1.y > surf.surface.p2.y;
				break;
			case Ordinal::NE:
				surf_concave = surf.surface.p2.y > surf.ghostp3.y 
					        && surf.surface.p2.y > surf.surface.p1.y;
				break;
			case Ordinal::SE:
				surf_concave = surf.surface.p1.y < surf.ghostp0.y 
					        && surf.surface.p1.y < surf.surface.p2.y;
				break;
			case Ordinal::SW:
				surf_concave = surf.surface.p2.y < surf.ghostp3.y 
					        && surf.surface.p2.y < surf.surface.p1.y;
				break;
			}

			if (surf_concave)
			{
				valleys_out[*ord] = true;
				has_valley |= true;
			}
		}
	}

	return !valid && has_valley;
}

CollisionAxis CollisionDiscrete::createEastWall(const AxisPreStep& initData) noexcept {

	CollisionAxis axis(initData);
    axis.contact.id = id;
	axis.contact.ortho_n    = Vec2f(1.f, 0.f);
	axis.contact.collider_n = Vec2f(1.f, 0.f);
	if (cQuad.material) {
		axis.contact.material = &cQuad.material->getSurface(Cardinal::E, cQuad.matFacing);
	}

	Vec2f pMid = math::rect_mid(cPrev) + collider_deltap;
	bool extend = wallCanExtend(axis, cQuad, Cardinal::E, pMid, cMid, tMid, tHalf, collision_time == Type::PrevFrame);
	bool has_valley = wallHasValley(axis, axes, axis_count, Cardinal::E, valleys);

	// if this is a oneway, invalidate it if the collider's previous position is not left of it
	if (collision_time == Type::CurrFrame && cQuad.isOneWay(Cardinal::E)) {
        float sliph = (cSlip.state == Collidable::SlipState::SlipHorizontal && cVel.x <= 0.f) ? cSlip.leeway : 0.f;
		axis.axisValid = math::rect_topleft(cPrev).x >= tPos.x + tArea.width - sliph;
	}

	axis.separationOffset = (extend ? cHalf.x : 0.f) + (has_valley ? VALLEY_FLATTEN_THRESH : 0.f);
	return axis;
}

CollisionAxis CollisionDiscrete::createWestWall(const AxisPreStep& initData) noexcept {

	CollisionAxis axis(initData);
    axis.contact.id = id;
	axis.contact.ortho_n    = Vec2f(-1.f, 0.f);
	axis.contact.collider_n = Vec2f(-1.f, 0.f);
	if (cQuad.material) {
		axis.contact.material = &cQuad.material->getSurface(Cardinal::W, cQuad.matFacing);
	}

	Vec2f pMid = math::rect_mid(cPrev) + collider_deltap;
	bool extend = wallCanExtend(axis, cQuad, Cardinal::W, pMid, cMid, tMid, tHalf, collision_time == Type::PrevFrame);
	bool has_valley = wallHasValley(axis, axes, axis_count, Cardinal::W, valleys);

	// if this is a oneway, invalidate it if the collider's previous position is not left of it
	if (collision_time == Type::CurrFrame && cQuad.isOneWay(Cardinal::W)) {
        float sliph = (cSlip.state == Collidable::SlipState::SlipHorizontal && cVel.x >= 0.f) ? cSlip.leeway : 0.f;
		axis.axisValid = math::rect_topright(cPrev).x <= tArea.left + sliph;
	}

	axis.separationOffset = (extend ? cHalf.x : 0.f) + (has_valley ? VALLEY_FLATTEN_THRESH : 0.f);
	return axis;
}

void CollisionDiscrete::initCollidableData(CollisionContext ctx) {
    if (collision_time == Type::CurrFrame) {
        cBox = ctx.collidable->getBox();
        cVel = ctx.collidable->get_local_vel();
    }
    else {
        cBox = ctx.collidable->getPrevBox();
        cVel = Vec2f{};
    }
    cPrev = ctx.collidable->getPrevBox();
    cMid  = math::rect_mid(cBox);
    cHalf = cBox.getSize() / 2.f;
    cSlip = ctx.collidable->getSlip();
}

}

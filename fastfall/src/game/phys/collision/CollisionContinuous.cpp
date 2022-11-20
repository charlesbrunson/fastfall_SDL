#include "fastfall/game/phys/collision/CollisionContinuous.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/util/math.hpp"

namespace ff {

CollisionContinuous::CollisionContinuous(CollisionContext ctx, CollisionID t_id)
    : id(t_id)
	, prevCollision(ctx, *ctx.collider->get_quad(t_id.quad), t_id, CollisionDiscrete::Type::PrevFrame)
	, currCollision(ctx, *ctx.collider->get_quad(t_id.quad), t_id, CollisionDiscrete::Type::CurrFrame)
    , prev_quad(*ctx.collider->get_quad(t_id.quad))
{
	evalContact(ctx, 0.0);
}

void CollisionContinuous::update(CollisionContext ctx, secs deltaTime)
{
    auto& curr_quad = *ctx.collider->get_quad(id.quad);
	if (deltaTime > 0.0) {

		// check if conditions are similar enough we can reuse the
		// collision data from last frame
		if (!ctx.collider->hasMoved()
			&& prev_quad == curr_quad)
		{
			prevCollision = std::move(currCollision);
			prevCollision.setPrevious();
			prevCollision.updateContact(ctx);
		}
		// else redo the collision from last frame
		else {
			prevCollision.reset(ctx, curr_quad, CollisionDiscrete::Type::PrevFrame);
		}

		currCollision.reset(ctx, curr_quad, CollisionDiscrete::Type::CurrFrame);
		evalContact(ctx, deltaTime);
	}
	else {
		currCollision.updateContact(ctx);
		evalContact(ctx, deltaTime);
	}
	prev_quad = curr_quad;
}

void CollisionContinuous::evalContact(CollisionContext ctx, secs deltaTime) {
	auto getRoot = [](float y0, float y1) {
		return -1.f * (y0 / (y1 - y0));
	};

	contact = {};
    contact.id = id;

	unsigned pCount = prevCollision.getAxisCount();
	unsigned cCount = currCollision.getAxisCount();

	assert(pCount == cCount);

	float firstExit = 1.f;
	float lastEntry = 0.f;

	const CollisionAxis* touchingAxis = nullptr;
	int touchingAxisNdx = 0;

	auto update_touch_axis = [&](float nRoot, const CollisionAxis* axis, int axis_ndx) 
	{
		if (nRoot >= 0.f && nRoot < 1.f && nRoot >= lastEntry)
		{
			if (touchingAxis)
			{
				if (nRoot == lastEntry
					&& touchingAxis->contact.separation > axis->contact.separation) 
				{
					touchingAxis = axis;
					touchingAxisNdx = axis_ndx;
				}
			}
			else {
				//lastEntry = nRoot;
				touchingAxis = axis;
				touchingAxisNdx = axis_ndx;
			}
		}
	};


	bool alwaysColliding = true;
	bool noCollision = false;

	const CollisionAxis* pAxis;
	const CollisionAxis* cAxis;

	bool pIntersects;
	bool cIntersects;

	float prev_min_valid_sep = FLT_MAX;
	float curr_min_valid_sep = FLT_MAX;

	for (unsigned i = 0; i < cCount; i++) {
		pAxis = &prevCollision.getCollisionAxis(i);
		cAxis = &currCollision.getCollisionAxis(i);

		if (pAxis->axisValid) {
			prev_min_valid_sep = std::min(pAxis->contact.separation, prev_min_valid_sep);
		}
		if (cAxis->axisValid) {
			curr_min_valid_sep = std::min(cAxis->contact.separation, curr_min_valid_sep);
		}

		pIntersects = pAxis->is_intersecting();
		cIntersects = cAxis->is_intersecting();
		alwaysColliding &= pIntersects && cIntersects;

		assert(pAxis->dir == cAxis->dir);
		assert(pAxis->contact.ortho_n == cAxis->contact.ortho_n);

		if (!pIntersects && !cIntersects) 
		{
			contact.hasContact = false;
			noCollision = true;
		}
		else if (pIntersects && !cIntersects) 
		{
			float root = getRoot(pAxis->contact.separation, cAxis->contact.separation);
			firstExit = std::min(firstExit, root);
		}
		else if (!pIntersects && cIntersects) 
		{
			float root = getRoot(pAxis->contact.separation, cAxis->contact.separation);
			lastEntry = std::max(lastEntry, root);

			if (cAxis->is_collider_valid())
			{
				update_touch_axis(root, cAxis, i);
			}
		}
	}

	bool hasTouchAxis = touchingAxis != nullptr;

	bool isDeparting = firstExit < 1.f;
	bool hasIntersect = firstExit >= lastEntry;

	bool lastAxisRepeatable = lastAxisCollided >= 0 && currCollision.getCollisionAxis(lastAxisCollided).applied;


	if (noCollision) 
	{
		// no collision at all
		contact = currCollision.getContact();
		lastAxisCollided = -1;
	}
	else if (hasTouchAxis && hasIntersect) 
	{
		// collision occured on a valid collision axis

		contact = touchingAxis->contact;
		contact.hasContact = touchingAxis->is_intersecting();

		if (contact.hasContact) 
		{
			// anti-tunneling measure
			// checks if opposite axis of this one
			// is still intersecting with collider
			// if not, object is tunneling though
			bool opposite_intersecting = false;
			for (int ndx = 0; ndx < currCollision.getAxisCount(); ndx++)
			{
				if (ndx != touchingAxisNdx
					&& currCollision.getCollisionAxis(ndx).dir == direction::opposite(touchingAxis->dir)
					&& currCollision.getCollisionAxis(ndx).contact.separation > 0)
				{
					opposite_intersecting = true;
					break;
				}
			}

			// anti-tunneling measure
			// if the collidable is no longer intersecting the collider,
			// and hasn't completely passed through this collider on this axis
			// require current collision has contact
			if (isDeparting && opposite_intersecting)
			{
				contact.hasContact &= currCollision.getContact().hasContact;
			}
		}
		

		contact.hasImpactTime = lastEntry > 0;
		contact.impactTime = lastEntry;
		lastAxisCollided = touchingAxisNdx;
	}
	else if (hasIntersect)
	{
		// complete intersection on both this and previous frame

		if (lastAxisRepeatable) {
			// if we know the axis we previously collided on
			// just do it again

			auto& axis = currCollision.getCollisionAxis(lastAxisCollided);
			contact = axis.contact;
			contact.hasContact = axis.is_intersecting();
		}
		else {
			// otherwise resort use current discrete contact
			contact = currCollision.getContact();
			lastAxisCollided = currCollision.getChosenAxis();
		}
	}

	if (deltaTime > 0.0) {
		velocity = ctx.collider->velocity;
	}

	contact.velocity = velocity;

    auto& curr_quad = *ctx.collider->get_quad(id.quad);
    contact.quad_valid = curr_quad.hasAnySurface();
    contact.id = id;

	evaluated = true;

	slipUpdate(ctx);
}


void CollisionContinuous::slipUpdate(CollisionContext ctx) {

	// vertical slip
	if (ctx.collidable->hasSlipV())
	{
		auto slip = getVerticalSlipContact(ctx.collidable->getSlip().leeway);
		if (slip) {
			contact = slip.value();
		}
	}

	// horizontal slip
	// TODO
}

std::optional<ContinuousContact> CollisionContinuous::getVerticalSlipContact(float leeway) {
	// contact must be evaluated first
	if (!evaluated)
		throw "contact must be evaluated first";

	// leeway must be != 0.f
	if (leeway <= 0.f)
		return std::nullopt;

	// slip only applicable on first contact
	if (!contact.hasContact  || !contact.hasImpactTime)
		return std::nullopt;

	// if we're trying to slip vertically then the 
	// contact must be horizontal
	if (contact.ortho_n.y != 0.f)
		return std::nullopt;

	const CollisionAxis* nAxis = nullptr;
	const CollisionAxis* sAxis = nullptr;
	for (unsigned ndx = 0u; ndx < currCollision.getAxisCount(); ndx++) {
		auto& axis = currCollision.getCollisionAxis(ndx);
		if (!nAxis && axis.dir == Cardinal::N) {
			nAxis = &axis;
		}
		if (!sAxis && axis.dir == Cardinal::S) {
			sAxis = &axis;
		}
	}

	bool canNorth = false;
	bool canSouth = false;

	if (nAxis 
		&& nAxis->is_collider_valid() 
		&& nAxis->contact.separation >= 0.f
		&& nAxis->contact.separation <= leeway) 
	{
		canNorth = true;
	}
	if (sAxis 
		&& sAxis->is_collider_valid() 
		&& sAxis->contact.separation >= 0.f
		&& sAxis->contact.separation <= leeway) 
	{
		canSouth = true;
	}

	if (canNorth && canSouth) {
		// select based on separation
		canNorth = sAxis->contact.separation > nAxis->contact.separation;
		canSouth = !canNorth;
	}

	if (canNorth && !canSouth) {
		ContinuousContact c = nAxis->contact;
		c.isSlip = true;
		c.hasImpactTime = contact.hasImpactTime;
		c.impactTime = contact.impactTime;
		return c;
	}
	else if (!canNorth && canSouth) {
		ContinuousContact c = sAxis->contact;
		c.isSlip = true;
		c.hasImpactTime = contact.hasImpactTime;
		c.impactTime = contact.impactTime;
		return c;
	}

	return std::nullopt;
}

}

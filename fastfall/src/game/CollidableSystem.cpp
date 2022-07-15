#include "fastfall/game/CollidableSystem.hpp"

namespace ff {

struct tracker_update_t 
{
	WorldState* st;
	Collidable* col;
	SurfaceTracker* tracker;
	ColliderRegion* collider = nullptr;
};


void tracker_move_with_platform(tracker_update_t& pack, CollidableOffsets& inout)
{
	auto& [state, collidable, tracker, collider] = pack;

	if (tracker->currentContact 
		&& tracker->settings.move_with_platforms)
	{
		PersistantContact& contact = *tracker->currentContact;
		if (collider && collider->hasMoved())
		{
			inout.position += math::projection(collider->getDeltaPosition(), contact.collider_n.lefthand(), true);
			inout.velocity += math::projection(collider->delta_velocity, contact.collider_n, true);
		}
	}
}

void tracker_set_speed(
	tracker_update_t& pack,
	float speed) 
{

	auto& [state, collidable, tracker, collider] = pack;

	if (tracker->has_contact())
	{
		Vec2f surf_unit = tracker->currentContact->collider_n.righthand();

		float surface_mag = 0.f;
		if (tracker->settings.use_surf_vel) {
			Vec2f surfaceVel = tracker->currentContact->getSurfaceVel();
			surface_mag = surfaceVel.x > 0 ? surfaceVel.magnitude() : -surfaceVel.magnitude();
		}

		Vec2f surfNV;
		if (collider && tracker->settings.move_with_platforms)
		{
			surfNV = math::projection(collider->velocity, tracker->currentContact->collider_n, true);
		}
		collidable->vel = surf_unit * (speed + surface_mag) + surfNV;
	}
}

void tracker_cap_speed(
	tracker_update_t& pack,
	secs deltaTime,
	CollidableOffsets& inout)
{
	auto& [state, collidable, tracker, collider] = pack;

	if (deltaTime > 0.0
		&& tracker->has_contact()
		&& tracker->settings.slope_sticking
		&& tracker->settings.max_speed > 0.f)
	{

		float surface_mag = 0.f;
		if (tracker->settings.use_surf_vel) {
			Vec2f surfaceVel = tracker->currentContact->getSurfaceVel();
			surface_mag = surfaceVel.x > 0 ? surfaceVel.magnitude() : -surfaceVel.magnitude();
		}

		float speed = *tracker->traverse_get_speed(collidable->vel);

		Vec2f acc_vec = math::projection(collidable->accel, tracker->currentContact->collider_n.righthand(), true);
		float acc_mag = acc_vec.magnitude();

		if (collidable->accel.x < 0.f) {
			acc_mag *= -1.f;
		}

		if (abs(speed + (acc_mag * deltaTime)) > tracker->settings.max_speed)
		{
			float n_speed = tracker->settings.max_speed * (speed < 0.f ? -1.f : 1.f);
			tracker_set_speed(pack, n_speed);
			inout.acceleration -= acc_vec;
		}
	}
}

void tracker_premove(
	tracker_update_t& pack,
	secs deltaTime,
	Vec2f& inout_next_pos,
	Vec2f& inout_surface_vel)
{
	auto& [state, collidable, tracker, collider] = pack;

	CollidableOffsets offsets;
	if (deltaTime > 0.0
		&& tracker->has_contact())
	{
		tracker_move_with_platform(pack, offsets);
		tracker_cap_speed(pack, deltaTime, offsets);
	}

	inout_next_pos += offsets.position;
	collidable->vel += offsets.velocity;
	collidable->accel += offsets.acceleration;

	if (tracker->has_contact()) {
		tracker->contact_time += deltaTime;
		tracker->air_time = 0.0;
	}
	else {
		tracker->air_time += deltaTime;
	}

	if (tracker->has_contact()) {
		inout_surface_vel += tracker->currentContact->getSurfaceVel();
	}
}


Vec2f tracker_slope_stick(
	tracker_update_t& pack,
	Vec2f wish_pos, 
	Vec2f prev_pos, 
	float left, 
	float right, 
	const PersistantContact& contact) 
{
	auto& [state, collidable, tracker, collider] = pack;

	// TODO: REFACTOR FOR ALL SURFACE DIRECTIONS

	Vec2f collider_offset;
	if (collider) {
		collider_offset = collider->getPosition();
	}

	static auto goLeft = [&](const ColliderSurface& surface) -> const ColliderSurface* {
		if (surface.surface.p1.x < surface.surface.p2.x) {
			if (auto* r = collider->get_surface(surface.prev)) {
				return r->surface.p1.x < r->surface.p2.x ? r : nullptr;
			}
		}
		else if (surface.surface.p1.x > surface.surface.p2.x) {
			if (auto* r = collider->get_surface(surface.next)) {
				return r->surface.p1.x > r->surface.p2.x ? r : nullptr;
			}
		}
		return nullptr;
	};
	static auto goRight = [&](const ColliderSurface& surface) -> const ColliderSurface* {


		// TODO Wall support?
		if (surface.surface.p1.x < surface.surface.p2.x) {
			if (auto* r = collider->get_surface(surface.next)) {
				return r->surface.p1.x < r->surface.p2.x ? r : nullptr;
			}
		}
		else if (surface.surface.p1.x > surface.surface.p2.x) {
			if (auto* r = collider->get_surface(surface.prev)) {
				return r->surface.p1.x > r->surface.p2.x ? r : nullptr;
			}
		}
		return nullptr;
	};

	const ColliderSurface* next = nullptr;

	bool goingLeft = false;
	bool goingRight = false;

	if (wish_pos.x > right && prev_pos.x <= right) {
		goingRight = true;
		next = goRight(contact.collider);
	}
	else if (wish_pos.x < left && prev_pos.x >= left) {
		goingLeft = true;
		next = goLeft(contact.collider);
	}

	else if (wish_pos.x > left && prev_pos.x <= left)
	{
		goingRight = true;
		next = &contact.collider;
	}
	else if (wish_pos.x < right && prev_pos.x >= right)
	{
		goingLeft = true;
		next = &contact.collider;
	}

	if (next) {

		Angle next_ang = math::angle(math::tangent(next->surface));
		Angle curr_ang = math::angle(contact.collider_n.righthand());
		Angle diff = next_ang - curr_ang;

		if (next_ang.radians() != curr_ang.radians()
			&& tracker->angle_range.within_range(next_ang - Angle::Degree(90.f))
			&& abs(diff.degrees()) < abs(tracker->settings.stick_angle_max.degrees()))
		{
			Vec2f hyp = wish_pos - ((goingRight ? next->surface.p1 : next->surface.p2) + collider_offset);
			Angle theta = math::angle(hyp) - math::angle(next->surface);

			// update velocity
			Angle gAng = math::angle(next->surface);
			if (goingLeft)
				gAng += Angle::Degree(180.f);

			Vec2f nVel;

			float slow = 1.f - tracker->settings.slope_stick_speed_factor * abs(diff.degrees() / tracker->settings.stick_angle_max.degrees());

			float vel_mag = collidable->vel.magnitude() * slow;
			nVel.x = cosf(gAng.radians()) * vel_mag;
			nVel.y = sinf(gAng.radians()) * vel_mag;

			//nVel = math::projection(owner->get_vel(), math::vector(next->surface));

			collidable->vel = nVel;

			if (theta.degrees() < 0.f && abs(diff.degrees()) < abs(tracker->settings.stick_angle_max.degrees())) {

				// update position
				float dist = hyp.magnitude() * sin(theta.radians());
				Vec2f offset = math::vector(next->surface).lefthand().unit();

				if (tracker->callbacks.on_stick)
					tracker->callbacks.on_stick(*next);

				return offset * dist;
			}
		}
	}
	return Vec2f{};
}


CollidableOffsets tracker_postmove(
	tracker_update_t& pack,
	Vec2f wish_pos, 
	Vec2f prev_pos)
{

	auto& [state, collidable, tracker, collider] = pack;

	std::optional<PersistantContact> local_contact
		= tracker->currentContact;

	bool local_has_contact = local_contact && local_contact->hasContact;

	CollidableOffsets out;

	Vec2f position = wish_pos;

	if (local_has_contact) {

		const auto& contact = local_contact.value();

		float left = std::min(contact.collider.surface.p1.x, contact.collider.surface.p2.x);
		float right = std::max(contact.collider.surface.p1.x, contact.collider.surface.p2.x);

		if (tracker->settings.slope_sticking 
			&& left < right) 
		{
			position += tracker_slope_stick(pack, wish_pos, prev_pos, left, right, contact);
		}
	}
	out.position = position - wish_pos;
	return out;

}


void CollidableSystem::precollision_update(WorldState& st, secs deltaTime)
{

	for (auto& collidable : st.list<Collidable>()) {

		Vec2f prev_pos = collidable.pos();
		Vec2f next_pos = collidable.pos();

		if (deltaTime > 0.0) {

			collidable.vel -= collidable.friction;
			collidable.accel = collidable.accel_accum;

			Vec2f surfaceVel = {};
			for (auto& tracker_id : collidable.trackers) {
				auto& tracker = st.get(tracker_id);

				ColliderRegion* collider = (tracker.has_contact() ? &st.get(tracker.currentContact->collider_id) : nullptr);

				tracker_update_t pack{
					.st = &st,
					.col = &collidable,
					.tracker = &tracker,
					.collider = collider
				};

				tracker_premove(pack, deltaTime, next_pos, surfaceVel);
			}

			collidable.vel += collidable.accel * deltaTime;
			collidable.vel.x = math::reduce(collidable.vel.x, collidable.decel_accum.x * (float)deltaTime, surfaceVel.x);
			collidable.vel.y = math::reduce(collidable.vel.y, collidable.decel_accum.y * (float)deltaTime, surfaceVel.y);

			next_pos += collidable.vel * deltaTime;

			// perform post move before applying gravity
			for (auto& tracker_id : collidable.trackers) {
				auto& tracker = st.get(tracker_id);

				ColliderRegion* collider = (tracker.has_contact() ? &st.get(tracker.currentContact->collider_id) : nullptr);

				tracker_update_t pack{
					.st = &st,
					.col = &collidable,
					.tracker = &tracker,
					.collider = collider
				};

				CollidableOffsets offsets = tracker_postmove(pack, next_pos, prev_pos);

				next_pos += offsets.position;
				collidable.vel += offsets.velocity;
				collidable.accel += offsets.acceleration;
			}

			// apply gravity
			collidable.vel += collidable.gravity * deltaTime;
			next_pos += collidable.gravity * deltaTime * deltaTime;

			// store velocity for postcollision
			collidable.precollision_vel = collidable.vel;

			// reset accumulators
			collidable.accel_accum = Vec2f{};
			collidable.decel_accum = Vec2f{};

			collidable.set_pos(next_pos);
		}
	}
}

}

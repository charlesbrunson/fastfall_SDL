#include "fastfall/game_v2/phys/collidable/SurfaceTracker.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game_v2/phys/ColliderRegion.hpp"

namespace ff {

SurfaceTracker::SurfaceTracker(ID<Collidable> t_owner, Angle ang_min, Angle ang_max, bool inclusive)
	: angle_range{ang_min, ang_max, inclusive}
    , owner_id(t_owner)
{

}

bool SurfaceTracker::has_contact() const noexcept {
	return currentContact && currentContact->hasContact;
};


bool SurfaceTracker::can_make_contact_with(const AppliedContact& contact) const noexcept
{
	Angle angle = math::angle(contact.collider_n);
	bool withinStickMax = true;
	if (currentContact)
	{
		Angle next_ang = math::angle(currentContact->collider_n.righthand());
		Angle curr_ang = math::angle(contact.collider_n.righthand());
		Angle diff = next_ang - curr_ang;

		withinStickMax = abs(diff.degrees()) < abs(settings.stick_angle_max.degrees());
	}
	return angle_range.within_range(angle) && withinStickMax;
}

void SurfaceTracker::process_contacts(
        poly_id_map<ColliderRegion>* colliders,
        std::vector<AppliedContact>& contacts)
{
	bool found = false;
	bool had_contact = currentContact.has_value();
	bool had_wall = wallContact.has_value();

	wallContact = std::nullopt;

	for (auto rit = contacts.rbegin(); rit != contacts.rend(); rit++) {
		auto& contact = *rit;

		// we are moving away from the contact
		if (contact.isSlip && math::dot(owner->get_vel(), contact.collider_n) > 0)
			continue;

		if (!found && can_make_contact_with(contact))
		{
			found = true;

			if (had_contact)
			{
				if (currentContact->id->collider != contact.id->collider)
				{
					end_touch(currentContact.value());
					start_touch(contact);
				}
			}
			else {

				start_touch(contact);
			}

			currentContact = contact;
			break;
		}
		else if (contact.collider_n.y == 0.f) {
			wallContact = contact;
		}
	}

	if (!found) {
		if (had_contact) {
			if (!do_slope_wall_stop(colliders, had_wall)) {
				end_touch(currentContact.value());
				currentContact = std::nullopt;
			}
		}
		else {
			currentContact = std::nullopt;
		}

	}

	if (!has_contact()) {
		contact_time = 0.0;
	}
}


Vec2f SurfaceTracker::calc_friction(Vec2f prevVel) {
	Vec2f friction;
	if (has_contact() 
		&& (!currentContact->hasImpactTime || contact_time > 0.0)
		&& settings.has_friction) 
	{
		Vec2f sVel = settings.use_surf_vel ? currentContact->surface_vel() : Vec2f{};
		Vec2f tangent = math::projection(prevVel - sVel, currentContact->collider_n.righthand(), true);
		Vec2f normal = math::projection(prevVel - sVel - currentContact->velocity, currentContact->collider_n, true);

		float Ft = tangent.magnitude();
		float Fn = normal.magnitude();

		float friction_mag;

		if (Ft < 2.f) {
			friction_mag = settings.surface_friction.stationary;
		}
		else if (Ft > 10.f) {
			friction_mag = settings.surface_friction.kinetic;
		}
		else {
			friction_mag = std::lerp(
				settings.surface_friction.stationary,
				settings.surface_friction.kinetic,
				(Ft - 2.f) / (10.f - 2.f));
		}

		float Ff = math::clamp(Fn * friction_mag, -Ft, Ft);

		friction = tangent.unit() * Ff;
	}
	return friction;
}

CollidableOffsets SurfaceTracker::premove_update(poly_id_map<ColliderRegion>* colliders, secs deltaTime) {

	CollidableOffsets out;

	if (deltaTime > 0.0 
		&& has_contact()) 
	{
		out = do_move_with_platform(colliders, out);
		out = do_max_speed(out, deltaTime);
	}

    if (has_contact()) {
        contact_time += deltaTime;
        air_time = 0.0;
    }
    else {
        air_time += deltaTime;
    }

	return out;
}

// ----------------------------

bool SurfaceTracker::do_slope_wall_stop(poly_id_map<ColliderRegion>* colliders, bool had_wall) noexcept {

	bool can_stop = settings.slope_wall_stop
		//&& !had_wall
		&& wallContact.has_value()
		&& !math::is_vertical(currentContact->collider_n)
		&& ((currentContact->collider_n.x < 0) == (wallContact->collider_n.x < 0))
		&& (math::dot(owner->get_vel(), currentContact->ortho_n) > 0);

	if (can_stop) {

		// correct velocity and position so we're still grounded
		owner->set_vel(Vec2f{});

		const ColliderRegion* region = colliders->get(currentContact->id->collider);

		if (settings.move_with_platforms && region
			&& region->hasMoved())
		{
			owner->set_vel(owner->get_vel() + math::projection(region->delta_velocity, currentContact->collider_n, true));
		}

		float X = owner->getPosition().x;
		Linef surface = currentContact->collider.surface;

		if (region) {
			surface.p1 += region->getPosition();
			surface.p2 += region->getPosition();
		}
		Vec2f intersect = math::intersection(
			surface,
			Linef{ Vec2f{X, 0.f}, Vec2f{X, 1.f} }
		);
		owner->setPosition(intersect, false);
	}
	return can_stop;

}

CollidableOffsets SurfaceTracker::do_move_with_platform(poly_id_map<ColliderRegion>* colliders, CollidableOffsets in) noexcept {

	if (currentContact && settings.move_with_platforms) 
	{
		AppliedContact& contact = currentContact.value();
		if (const ColliderRegion* region = colliders->get(currentContact->id->collider);
			region && region->hasMoved()) 
		{
			in.position += math::projection(region->getDeltaPosition(), contact.collider_n.lefthand(), true);
			in.velocity += math::projection(region->delta_velocity, 	contact.collider_n, 			true);
		}
	}
	return in;
}

CollidableOffsets SurfaceTracker::do_max_speed(CollidableOffsets in, secs deltaTime) noexcept {

	if (deltaTime > 0.0 
		&& has_contact() 
		&& settings.slope_sticking 
		&& settings.max_speed > 0.f) 
	{
		float speed = *traverse_get_speed();
		Vec2f acc_vec = math::projection(owner->get_acc(), currentContact->collider_n.righthand(), true);
		float acc_mag = acc_vec.magnitude();

		if (owner->get_acc().x < 0.f) {
			acc_mag *= -1.f;
		}

		if (abs(speed + (acc_mag * deltaTime)) > settings.max_speed) {
			traverse_set_speed(settings.max_speed * (speed < 0.f ? -1.f : 1.f));
			in.acceleration -= acc_vec;
		}
	}
	return in;
}

// ----------------------------

Vec2f SurfaceTracker::do_slope_stick(poly_id_map<ColliderRegion>* colliders, Vec2f wish_pos, Vec2f prev_pos, float left, float right) const noexcept {

	// TODO: REFACTOR FOR ALL SURFACE DIRECTIONS

	Vec2f regionOffset;
    const ColliderRegion* region = colliders->get(currentContact->id->collider);
	if (region) {
		regionOffset = region->getPosition();
	}

	static auto goLeft = [](const ColliderRegion* region, const ColliderSurface& surface) -> const ColliderSurface* {

		if (region) {
			if (surface.surface.p1.x < surface.surface.p2.x && surface.prev_id) {
				if (auto* r = region->get_surface_collider(*surface.prev_id)) {
					return r->surface.p1.x < r->surface.p2.x ? r : nullptr;
				}
			}
			else if (surface.surface.p1.x > surface.surface.p2.x && surface.next_id) {
				if (auto* r = region->get_surface_collider(*surface.next_id)) {
					return r->surface.p1.x > r->surface.p2.x ? r : nullptr;
				}
			}
		}
		return nullptr;
	};
	static auto goRight = [](const ColliderRegion* region, const ColliderSurface& surface) -> const ColliderSurface* {

		// TODO Wall support?
		if (region) {
			if (surface.surface.p1.x < surface.surface.p2.x && surface.next_id) {
				if (auto* r = region->get_surface_collider(*surface.next_id)) {
					return r->surface.p1.x < r->surface.p2.x ? r : nullptr;
				}
			}
			else if (surface.surface.p1.x > surface.surface.p2.x && surface.prev_id) {
				if (auto* r = region->get_surface_collider(*surface.prev_id)) {
					return r->surface.p1.x > r->surface.p2.x ? r : nullptr;
				}
			}
		}
		return nullptr;
	};

	const ColliderSurface* next = nullptr;

	bool goingLeft = false;
	bool goingRight = false;

	if (wish_pos.x > right && prev_pos.x <= right) {
		goingRight = true;
		next = goRight(region, currentContact->collider);
	}
	else if (wish_pos.x < left && prev_pos.x >= left) {
		goingLeft = true;
		next = goLeft(region, currentContact->collider);
	}

	else if (wish_pos.x > left && prev_pos.x <= left)
	{
		goingRight = true;
		next = &currentContact->collider;
	}
	else if (wish_pos.x < right && prev_pos.x >= right)
	{
		goingLeft = true;
		next = &currentContact->collider;
	}

	if (next) {

		Angle next_ang = math::angle(math::tangent(next->surface));
		Angle curr_ang = math::angle(currentContact->collider_n.righthand());
		Angle diff = next_ang - curr_ang;

		if (next_ang.radians() != curr_ang.radians()
			&& angle_range.within_range(next_ang - Angle::Degree(90.f))
			&& abs(diff.degrees()) < abs(settings.stick_angle_max.degrees()))
		{
			Vec2f hyp = wish_pos - ((goingRight ? next->surface.p1 : next->surface.p2) + regionOffset);
			Angle theta = math::angle(hyp) - math::angle(next->surface);

			// update velocity
			Angle gAng = math::angle(next->surface);
			if (goingLeft)
				gAng += Angle::Degree(180.f);

			Vec2f nVel;

			float slow = 1.f - settings.slope_stick_speed_factor * abs(diff.degrees() / settings.stick_angle_max.degrees());

			float vel_mag = owner->get_vel().magnitude() * slow;
			nVel.x = cosf(gAng.radians()) * vel_mag;
			nVel.y = sinf(gAng.radians()) * vel_mag;

			owner->set_vel(nVel);

			if (theta.degrees() < 0.f && abs(diff.degrees()) < abs(settings.stick_angle_max.degrees())) {

				// update position
				float dist = hyp.magnitude() * sin(theta.radians());
				Vec2f offset = math::vector(next->surface).lefthand().unit();

				if (callbacks.on_stick)
					callbacks.on_stick(*next);

				return offset * dist;
			}
		}
	}
	return Vec2f{};
}

CollidableOffsets SurfaceTracker::postmove_update(
        poly_id_map<ColliderRegion>* colliders,
        Vec2f wish_pos,
        Vec2f prev_pos
    ) const
{
	CollidableOffsets out;
	Vec2f position = wish_pos;

	if (has_contact()) {

		float left = std::min(currentContact->collider.surface.p1.x, currentContact->collider.surface.p2.x);
		float right = std::max(currentContact->collider.surface.p1.x, currentContact->collider.surface.p2.x);

		if (settings.slope_sticking && left < right) {
			position += do_slope_stick(colliders, wish_pos, prev_pos, left, right);
		}
	}
	out.position = position - wish_pos;
	return out;
	
}

void SurfaceTracker::start_touch(AppliedContact& contact) {

	if (settings.move_with_platforms) {
		owner->set_vel(owner->get_vel() - math::projection(contact.velocity, contact.collider_n.lefthand(), true));
	}

	if (callbacks.on_start_touch)
		callbacks.on_start_touch(contact);
}

void SurfaceTracker::end_touch(AppliedContact& contact) {

	if (settings.move_with_platforms) {
		// have to avoid collider velocity being double-applied
		bool still_touching = false;
		for (auto& contact_ : owner->get_contacts()) {
			if (contact.id->collider == contact_.id->collider) {
				still_touching = true;
			}
		}
		if (!still_touching) {
			owner->set_vel(owner->get_vel() + math::projection(contact.velocity, contact.collider_n.lefthand(), true));
		}
	}

	if (callbacks.on_end_touch)
		callbacks.on_end_touch(contact);
}


void SurfaceTracker::traverse_set_speed(float speed) {
	if (has_contact()) 
	{
		Vec2f surf_unit = currentContact->collider_n.righthand();

		float surface_mag = 0.f;
		if (settings.use_surf_vel) {
			Vec2f surfaceVel = currentContact->surface_vel();
			surface_mag = surfaceVel.x > 0 ? surfaceVel.magnitude() : -surfaceVel.magnitude();
		}

		Vec2f surfNV;
		if (settings.move_with_platforms && currentContact->velocity != Vec2f{})
		{
            surfNV = math::projection(currentContact->velocity, currentContact->collider_n, true);
		}
		owner->set_vel(surf_unit * (speed + surface_mag) + surfNV);
	}
}

void SurfaceTracker::traverse_add_accel(float accel) {
	if (has_contact()) {
		Vec2f surf_unit = currentContact->collider_n.righthand();
		owner->add_accel(surf_unit * accel);
	}
}

void SurfaceTracker::traverse_add_decel(float decel) {
	if (has_contact()) {
		Vec2f surf_unit = currentContact->collider_n.righthand();
		owner->add_decel(Vec2f{ abs(surf_unit.x), abs(surf_unit.y) } *decel);
	}
}

std::optional<float> SurfaceTracker::traverse_get_speed() {
	std::optional<float> speed;
	if (has_contact()) {
		Vec2f surfVel = (settings.use_surf_vel ? currentContact->surface_vel() : Vec2f{});
		Vec2f surf = currentContact->collider_n.righthand();
		Vec2f proj = math::projection(owner->get_vel(), surf, true) - surfVel;

		if (proj.x == 0.f) {
			speed = 0.f;
		}
		else if (proj.x < 0.f != surf.x < 0.f) {
			speed = -proj.magnitude();
		}
		else {
			speed = proj.magnitude();
		}
	}
	return speed;
}

void SurfaceTracker::firstCollisionWith(const AppliedContact& contact)
{
	// ugly hack to let surface tracker stick on the first frame
	if (!has_contact() 
		&& can_make_contact_with(contact)
		&& settings.slope_sticking
		&& contact.stickOffset != 0.f)
	{
		AppliedContact pc{contact};
		start_touch(pc);

		owner->setPosition(owner->getPosition() + (contact.ortho_n * contact.stickOffset), false);

		float vmag = owner->get_vel().magnitude();
		owner->set_vel(vmag * math::projection(owner->get_vel(), math::vector(contact.stickLine)).unit());

	}
}

void SurfaceTracker::force_end_contact() {
    if (currentContact.has_value()) {
        end_touch(currentContact.value());
        currentContact = std::nullopt;
        contact_time = 0.0;
    }
}

}

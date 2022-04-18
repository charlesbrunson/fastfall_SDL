#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

#include "fastfall/game/InstanceInterface.hpp"
#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game/phys/ColliderRegion.hpp"

namespace ff {

SurfaceTracker::SurfaceTracker(Angle ang_min, Angle ang_max, bool inclusive) 
	: angle_min(ang_min)
	, angle_max(ang_max)
	, angle_inclusive(inclusive)
{

}

bool SurfaceTracker::has_contact() const noexcept {
	return currentContact && currentContact->hasContact;
};

void SurfaceTracker::process_contacts(std::vector<PersistantContact>& contacts) {

	bool found = false;
	bool had_contact = currentContact.has_value();

	wallYadj = 0.f;

	bool had_wall = wallContact.has_value();

	wallContact = std::nullopt;

	for (auto rit = contacts.rbegin(); rit != contacts.rend(); rit++) {
		auto& contact = *rit;
		if (contact.isSlip)
			continue;

		Angle angle = math::angle(contact.collider_normal);

		bool withinStickMax = true;
		if (had_contact)
		{
			Angle next_ang = math::angle(currentContact->collider.surface);
			Angle curr_ang = math::angle(contact.collider.surface);
			Angle diff = next_ang - curr_ang;

			withinStickMax = abs(diff.degrees()) < abs(settings.stick_angle_max.degrees());
		}

		if (!found && angle.isBetween(angle_min, angle_max, angle_inclusive) && withinStickMax)
		{

			found = true;


			if (had_contact)
			{
				if (currentContact->region != contact.region)
				{
					end_touch(currentContact.value());
					start_touch(contact);
				}
			}
			else if (!had_contact) {

				start_touch(contact);
			}

			currentContact = contact;
			break;
		}
		else if (contact.collider_normal.y == 0.f) {
			wallContact = contact;
		}
	}

	if (!found) {
		if (had_contact) {
			if (!do_slope_wall_stop(had_wall)) {
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


Vec2f SurfaceTracker::get_friction(Vec2f prevVel) {
	Vec2f friction;
	if (has_contact() 
		&& (!currentContact->hasImpactTime || contact_time > 0.0)
		&& settings.has_friction) 
	{
		Vec2f sVel = settings.use_surf_vel ? currentContact->getSurfaceVel() : Vec2f{};
		Vec2f tangent = math::projection(prevVel - sVel, currentContact->collider_normal.righthand(), true);
		Vec2f normal = math::projection(prevVel - sVel - currentContact->velocity, currentContact->collider_normal, true);

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

CollidableOffsets SurfaceTracker::premove_update(secs deltaTime) {
	//Vec2f acceleration;

	CollidableOffsets out;

	if (deltaTime > 0.0) {
		if (!has_contact())
			return {};

		out = do_move_with_platform(out);

		out.acceleration += do_max_speed(deltaTime);
	}

	//return acceleration;
	return out;
}

// ----------------------------

bool SurfaceTracker::do_slope_wall_stop(bool had_wall) noexcept {

	bool can_stop = settings.slope_wall_stop
		//&& !had_wall
		&& wallContact.has_value()
		&& !math::is_vertical(currentContact->collider_normal)
		&& ((currentContact->collider_normal.x < 0) == (wallContact->collider_normal.x < 0))
		&& (math::dot(owner->get_vel(), currentContact->ortho_normal) > 0);

	if (can_stop) {

		// correct velocity and position so we're still grounded
		owner->set_vel(Vec2f{});

		const auto* region = currentContact->region;

		if (settings.move_with_platforms && region
			&& region->getPosition() != region->getPrevPosition()) 
		{
			owner->set_vel(owner->get_vel() + math::projection(region->delta_velocity, currentContact->collider_normal, true));
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

CollidableOffsets SurfaceTracker::do_move_with_platform(CollidableOffsets in) noexcept {

	//bool did_move = false;

	if (settings.move_with_platforms) {
		PersistantContact& contact = currentContact.value();
		if (const auto* region = currentContact->region) {
			if (region->getPosition() != region->getPrevPosition()) {
				Vec2f offset = region->getPosition() - region->getPrevPosition();

				Vec2f move_off = math::projection(offset, contact.collider_normal.lefthand(), true);

				in.position += move_off;
				in.velocity += math::projection(region->delta_velocity, contact.collider_normal, true);

				//owner->move(move_off, false);
				//owner->set_vel(owner->get_vel() + math::projection(region->delta_velocity, contact.collider_normal, true));
				//did_move = true;
			}
		}
	}
	return in;
}

Vec2f SurfaceTracker::do_max_speed(secs deltaTime) noexcept {

	//Vec2f accel{};
	Vec2f decel{};

	if (deltaTime > 0.0 && has_contact() && settings.slope_sticking && settings.max_speed > 0.f) {

		float surface_mag = 0.f;
		if (settings.use_surf_vel) {
			Vec2f surfaceVel = currentContact->getSurfaceVel();
			surface_mag = surfaceVel.x > 0 ? surfaceVel.magnitude() : -surfaceVel.magnitude();
		}

		float speed = traverse_get_speed();

		Vec2f acc_vec = math::projection(owner->get_acc(), math::vector(currentContact->collider.surface));
		float acc_mag = acc_vec.magnitude();
		if (owner->get_acc().x < 0.f) {
			acc_mag *= -1.f;
		}

		if (abs(speed + (acc_mag * deltaTime)) > settings.max_speed) {
			traverse_set_speed(settings.max_speed * (speed < 0.f ? -1.f : 1.f));
			decel -= acc_vec;
		}
	}
	//return accel;
	return decel;
}

// ----------------------------

CollidableOffsets SurfaceTracker::postmove_update(Vec2f wish_pos, Vec2f prev_pos, secs deltaTime, std::optional<PersistantContact> contact_override) const {

	std::optional<PersistantContact> local_contact;

	if (contact_override) {
		local_contact = *contact_override;
	}
	else if (currentContact) {
		local_contact = *currentContact;
	}
	bool local_has_contact = local_contact && local_contact->hasContact;


	CollidableOffsets out;

	Vec2f position = wish_pos;

	if (local_has_contact && deltaTime > 0.0) {

		Vec2f regionOffset;
		if (const auto* region = local_contact->region) {
			regionOffset = region->getPosition();
		}

		static auto goLeft = [](const ColliderSurface& surface) -> const ColliderSurface* {
			if (surface.surface.p1.x < surface.surface.p2.x) {
				if (auto* r = surface.prev) {
					return r->surface.p1.x < r->surface.p2.x ? r : nullptr;
				}
			}
			else if (surface.surface.p1.x > surface.surface.p2.x) {
				if (auto* r = surface.next) {
					return r->surface.p1.x > r->surface.p2.x ? r : nullptr;
				}
			}
			return nullptr;
		};
		static auto goRight = [](const ColliderSurface& surface) -> const ColliderSurface* {


			// TODO Wall support?
			if (surface.surface.p1.x < surface.surface.p2.x) {
				if (auto* r = surface.next) {
					return r->surface.p1.x < r->surface.p2.x ? r : nullptr;
				}
			}
			else if (surface.surface.p1.x > surface.surface.p2.x) {
				if (auto* r = surface.prev) {
					return r->surface.p1.x > r->surface.p2.x ? r : nullptr;
				}
			}
			return nullptr;
		};


		const auto& contact = local_contact.value();

		float left = std::min(contact.collider.surface.p1.x, contact.collider.surface.p2.x);
		float right = std::max(contact.collider.surface.p1.x, contact.collider.surface.p2.x);

		if (settings.slope_sticking && left < right) {

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

			if (next) {

				Angle next_ang = math::angle(next->surface);
				Angle curr_ang = math::angle(contact.collider.surface);
				Angle diff = next_ang - curr_ang;

				if (next_ang.radians() != curr_ang.radians()
					&& is_angle_in_range(next_ang - Angle::Degree(90.f))
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

					//nVel = math::projection(owner->get_vel(), math::vector(next->surface));

					owner->set_vel(nVel);

					if (theta.degrees() < 0.f && abs(diff.degrees()) < abs(settings.stick_angle_max.degrees())) {

						// update position
						float dist = hyp.magnitude() * sin(theta.radians());
						Vec2f offset = math::vector(next->surface).lefthand().unit();

						position += offset * dist;

						if (callback_on_stick)
							callback_on_stick(*next);

					}
				}
			}
		}

	}

	if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_CONTACT))
	{
		
		if (wish_pos != position)
		{
			auto& draw1 = createDebugDrawable<ShapeCircle, debug_draw::Type::COLLISION_CONTACT>(
				(const void*)this,
				wish_pos,
				1.f,
				4,
				Color::Transparent,
				Color::Red);

			auto& draw2 = createDebugDrawable<ShapeCircle, debug_draw::Type::COLLISION_CONTACT>(
				(const void*)this,
				position,
				1.f,
				4,
				Color::Transparent,
				Color::Yellow);

			auto& draw3 = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_CONTACT>(
				(const void*)this,
				Primitive::LINES,
				4
				);


			draw3[0].pos = wish_pos;
			draw3[1].pos = position;

			draw3[0].color = Color::Red;
			draw3[1].color = Color::Yellow;


			draw3[2].pos = Vec2f{ owner->getPrevBox().getPosition() } + Vec2f{ owner->getPrevBox().getSize().x * 0.5f, owner->getPrevBox().getSize().y };
			draw3[3].pos = wish_pos;

			draw3[2].color = Color::Red;
			draw3[3].color = Color::Red;
			
		}
		else {
			debug_draw::repeat((const void*)this, Vec2f{});
		}
		
	}
	out.position = position - wish_pos;
	return out;
}

void SurfaceTracker::start_touch(PersistantContact& contact) {

	if (settings.move_with_platforms) {
		owner->set_vel(owner->get_vel() - math::projection(contact.velocity, contact.collider_normal.lefthand(), true));
	}

	if (callback_start_touch)
		callback_start_touch(contact);
}

void SurfaceTracker::end_touch(PersistantContact& contact) {

	if (settings.move_with_platforms) {
		// have to avoid collider velocity being double-applied
		bool still_touching = false;
		for (auto& contact_ : owner->get_contacts()) {
			if (contact.region == contact_.region) {
				still_touching = true;
			}
		}
		if (!still_touching) {
			owner->set_vel(owner->get_vel() + math::projection(contact.velocity, contact.collider_normal.lefthand(), true));
		}
	}


	if (callback_end_touch)
		callback_end_touch(contact);
}

/*
void SurfaceTracker::traverse_set_max_speed(float speed) {
	max_speed = speed;
}
*/

void SurfaceTracker::traverse_set_speed(float speed) {
	if (!has_contact()) {
		owner->set_velX(speed);
		return;
	}

	Vec2f surf_unit = math::vector(currentContact->collider.surface).unit();

	float surface_mag = 0.f;
	if (settings.use_surf_vel) {
		Vec2f surfaceVel = currentContact->getSurfaceVel();
		surface_mag = surfaceVel.x > 0 ? surfaceVel.magnitude() : -surfaceVel.magnitude();
	}

	if (settings.move_with_platforms && has_contact()) {

		Vec2f surfNV;

		if (const auto* region = currentContact->region) {
			surfNV = math::projection(region->velocity, currentContact->collider_normal, true);
		}

 		owner->set_vel(surf_unit * (speed + surface_mag) + surfNV);
	}
	else {
		owner->set_vel(surf_unit * (speed + surface_mag));

	}
}

void SurfaceTracker::traverse_add_accel(float accel) {
	if (!has_contact()) {
		owner->add_accelX(accel);
		return;
	}

	Vec2f surf_unit = math::vector(currentContact->collider.surface).unit();

	owner->add_accel(surf_unit * accel);
}

void SurfaceTracker::traverse_add_decel(float decel) {
	if (!has_contact()) {
		owner->add_decelX(decel);
		return;
	}

	Vec2f surf_unit = math::vector(currentContact->collider.surface).unit();
	owner->add_decel(Vec2f{ abs(surf_unit.x), abs(surf_unit.y) } *decel);
}

float SurfaceTracker::traverse_get_speed() {
	if (!has_contact()) {
		return owner->get_vel().x;
	}

	Vec2f surfVel = (settings.use_surf_vel ? currentContact->getSurfaceVel() : Vec2f{});

	Vec2f surf = math::vector(currentContact->collider.surface);
	Vec2f proj = math::projection(owner->get_vel(), surf) - surfVel;

	if (proj.x == 0.f) {
		return 0.f;
	}
	else if (proj.x < 0.f != surf.x < 0.f) {
		return -proj.magnitude();
	}
	else {
		return proj.magnitude();
	}
}

}

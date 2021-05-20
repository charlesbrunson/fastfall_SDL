#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

#include "fastfall/game/phys//Collidable.hpp"
#include "fastfall/game/phys//ColliderRegion.hpp"

//#include "fastfall/game/DebugDraw.hpp"

namespace ff {

SurfaceTracker::SurfaceTracker(GameContext game_context, Angle ang_min, Angle ang_max, bool inclusive) :
	context(game_context),
	angle_min(ang_min),
	angle_max(ang_max),
	angle_inclusive(inclusive)
{

}

SurfaceTracker::~SurfaceTracker() {
	if (owner)
		owner->remove_tracker(*this);

	owner = nullptr;
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

	for (auto& contact : contacts) {
		Angle angle = math::angle(contact.collider_normal);

		if (!found && angle.isBetween(angle_min, angle_max, angle_inclusive))
		{

			found = true;

			if (had_contact
				&& currentContact->collider_id.value != contact.collider_id.value)
			{
				end_touch(currentContact.value());
				start_touch(contact);
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
		duration = 0.0;
	}
}


Vec2f SurfaceTracker::get_friction(Vec2f prevVel) {
	Vec2f friction;
	if (has_contact() 
		&& (!currentContact->hasImpactTime || duration > 0.0) 
		&& has_friction) 
	{

		Vec2f tangent = math::projection(prevVel, currentContact->collider_normal.righthand(), true);
		Vec2f normal = math::projection(prevVel - currentContact->velocity, currentContact->collider_normal, true);

		float Ft = tangent.magnitude();
		float Fn = normal.magnitude();

		float friction_mag;

		if (Ft < 2.f) {
			friction_mag = surface_friction.stationary;
		}
		else if (Ft > 10.f) {
			friction_mag = surface_friction.kinetic;
		}
		else {
			friction_mag = std::lerp(
				surface_friction.stationary,
				surface_friction.kinetic,
				(Ft - 2.f) / (10.f - 2.f));
		}

		float Ff = math::clamp(Fn * friction_mag, -Ft, Ft);

		friction = tangent.unit() * Ff;
	}
	else {
		volatile int x = 0;
	}
	return friction;
}

Vec2f SurfaceTracker::premove_update(secs deltaTime) {
	Vec2f acceleration;

	if (!has_contact())
		return acceleration;

	do_move_with_platform();

	acceleration += do_slope_sticking(deltaTime);

	return acceleration;
}

// ----------------------------

bool SurfaceTracker::do_slope_wall_stop(bool had_wall) noexcept {

	bool can_stop = slope_wall_stop
		&& !had_wall
		&& wallContact.has_value()
		&& !math::is_vertical(currentContact->collider_normal)
		&& ((currentContact->collider_normal.x < 0) == (wallContact->collider_normal.x < 0))
		&& (math::dot(owner->get_vel(), currentContact->ortho_normal) > 0);

	if (can_stop) {

		// correct velocity and position so we're still grounded
		owner->set_vel(Vec2f{ 0.f, 0.f });
		if (move_with_platforms) {
			if (const auto* region = context.collision().get_region(currentContact->collider_id)) {
				if (region->getPosition() != region->getPrevPosition()) {
					owner->set_vel(owner->get_vel() + math::projection(region->delta_velocity, currentContact->collider_normal, true));
				}
			}
		}

		float X = owner->getPosition().x;
		Linef surface = currentContact->collider.surface;

		if (const auto* region = context.collision().get_region(currentContact->collider_id)) {
			surface.p1 += region->getPosition();
			surface.p2 += region->getPosition();
		}
		Vec2f intersect = math::intersection(
			surface,
			Linef{ Vec2f{X, 0.f}, Vec2f{X, 1.f} }
		);
		owner->setPosition(intersect);
	}
	return can_stop;

}

bool SurfaceTracker::do_move_with_platform() noexcept {

	bool did_move = false;

	if (move_with_platforms) {
		PersistantContact& contact = currentContact.value();
		if (const auto* region = context.collision().get_region(contact.collider_id)) {
			if (region->getPosition() != region->getPrevPosition()) {
				Vec2f offset = region->getPosition() - region->getPrevPosition();

				owner->move(math::projection(offset, contact.collider_normal.lefthand(), true));
				owner->set_vel(owner->get_vel() + math::projection(region->delta_velocity, contact.collider_normal, true));
				did_move = true;
			}
		}
	}
	return did_move;
}

Vec2f SurfaceTracker::do_slope_sticking(secs deltaTime) noexcept {

	Vec2f accel{};

	if (slope_sticking && max_speed > 0.f) {

		float speed = traverse_get_speed();

		Vec2f acc_vec = math::projection(owner->get_acc(), math::vector(currentContact->collider.surface));
		float acc_mag = acc_vec.magnitude();
		if (owner->get_acc().x < 0.f) {
			acc_mag *= -1.f;
		}

		if (abs(speed + (acc_mag * deltaTime)) > max_speed) {
			traverse_set_speed(max_speed * (speed < 0.f ? -1.f : 1.f));
			accel -= acc_vec;
		}
	}
	return accel;
}

// ----------------------------

Vec2f SurfaceTracker::postmove_update(Vec2f wish_pos, secs deltaTime) {
	if (!has_contact())
		return wish_pos;


	Vec2f regionOffset;
	if (const auto* region = context.collision().get_region(currentContact->collider_id)) {
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

	Vec2f position = wish_pos;

	auto& contact = currentContact.value();

	float left = std::min(contact.collider.surface.p1.x, contact.collider.surface.p2.x);
	float right = std::max(contact.collider.surface.p1.x, contact.collider.surface.p2.x);

	if (slope_sticking && left < right) {

		const ColliderSurface* next = nullptr;

		bool goingLeft = false;
		bool goingRight = false;

		if (wish_pos.x > right) {
			goingRight = true;
			next = goRight(contact.collider);
		}
		else if (wish_pos.x < left) {
			goingLeft = true;
			next = goLeft(contact.collider);
		}

		if (next) {
			Angle diff = math::angle(next->surface) - math::angle(contact.collider.surface);

			//float dir1 = math::vector(contact.collider.surface).y;
			//float dir2 = math::vector(next->surface).y;

			if (math::angle(next->surface) != math::angle(contact.collider.surface) &&
				is_angle_in_range(math::angle(next->surface) - Angle::Degree(90.f)) &&
				//(dir1 == 0.f || dir2 == 0.f || ((dir1 < 0.f) == (dir2 < 0.f))) &&
				abs(diff.degrees()) < abs(stick_angle_max.degrees())
				) {

				Vec2f hyp = wish_pos - ((goingRight ? next->surface.p1 : next->surface.p2) + regionOffset);
				Angle theta = math::angle(hyp) - math::angle(next->surface);

				// update velocity
				Angle gAng = math::angle(next->surface);
				if (goingLeft)
					gAng += Angle::Degree(180.f);

				Vec2f nVel;
				float vel_mag = owner->get_vel().magnitude();
				nVel.x = cosf(gAng.radians()) * vel_mag;
				nVel.y = sinf(gAng.radians()) * vel_mag;
				owner->set_vel(nVel);

				if (theta.degrees() < 0.f && abs(diff.degrees()) < abs(stick_angle_max.degrees())) {

					// update position
					float dist = hyp.magnitude() * sin(theta.radians());
					Vec2f offset = math::vector(next->surface).lefthand().unit();

					position += offset * dist * 2.f;

					if (callback_on_stick)
						callback_on_stick(*next);

				}
			}
		}
	}

	return position;
}

void SurfaceTracker::start_touch(PersistantContact& contact) {

	if (move_with_platforms) {
		owner->set_vel(owner->get_vel() - math::projection(contact.velocity, contact.collider_normal.lefthand(), true));
	}

	if (callback_start_touch)
		callback_start_touch(contact);
}

void SurfaceTracker::end_touch(PersistantContact& contact) {

	if (move_with_platforms) {
		// have to avoid collider velocity being double-applied
		bool still_touching = false;
		for (auto& contact : owner->get_contacts()) {
			if (contact.collider_id.value == contact.collider_id.value) {
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

void SurfaceTracker::traverse_set_max_speed(float speed) {
	max_speed = speed;
}

void SurfaceTracker::traverse_set_speed(float speed) {
	if (!has_contact()) {
		owner->set_velX(speed);
		return;
	}

	Vec2f surf_unit = math::vector(currentContact->collider.surface).unit();

	if (move_with_platforms && has_contact()) {

		Vec2f surfNV;

		if (const auto* region = context.collision().get_region(currentContact->collider_id)) {
			surfNV = math::projection(region->velocity, currentContact->collider_normal, true);
			//LOG_INFO(surfNV.to_string());
		}

		owner->set_vel(surf_unit * speed + surfNV);
	}
	else {
		owner->set_vel(surf_unit * speed);

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

	Vec2f surf = math::vector(currentContact->collider.surface);
	Vec2f proj = math::projection(owner->get_vel(), surf);

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
#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game/phys/ColliderRegion.hpp"

#include <set>
#include <algorithm>

namespace ff {

SurfaceTracker::SurfaceTracker(Collidable* t_owner, Angle ang_min, Angle ang_max, bool inclusive)
	: angle_range{ang_min, ang_max, inclusive}
    , owner(t_owner)
{

}

bool SurfaceTracker::has_contact() const noexcept {
	return currentContact && currentContact->hasContact;
};

bool SurfaceTracker::has_contact_with(ID<ColliderRegion> collider) const noexcept {
   return has_contact() && currentContact->id.has_value() && currentContact->id->collider == collider;
}


bool SurfaceTracker::can_make_contact_with(Vec2f collider_normal) const noexcept
{
    Angle angle = math::angle(collider_normal);
    bool withinStickMax = true;
    if (currentContact)
    {
        Angle next_ang = math::angle(currentContact->collider_n.righthand());
        Angle curr_ang = math::angle(collider_normal.righthand());
        Angle diff = next_ang - curr_ang;

        withinStickMax = abs(diff.degrees()) < abs(settings.stick_angle_max.degrees());
    }
    return angle_range.within_range(angle) && withinStickMax;
}

bool SurfaceTracker::can_make_contact_with(const AppliedContact& contact) const noexcept
{
    return can_make_contact_with(contact.collider_n);
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
		if (contact.isSlip && math::dot(owner->get_global_vel(), contact.collider_n) > 0)
			continue;

		if (!found && can_make_contact_with(contact))
		{
			found = true;
            const ColliderRegion* region = contact.id ? colliders->get(contact.id->collider) : nullptr;
            contact.velocity = region->velocity; // update vel to region vel
			if (had_contact)
			{
				if (currentContact->id->collider != contact.id->collider)
				{
                    end_touch(currentContact.value());
					start_touch( contact);
				}
                else {
                    continue_touch(contact);
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
            // need to update the current contact for the region's current position
            const ColliderRegion* region =
                    currentContact && currentContact->id
                    ? colliders->get(currentContact->id->collider)
                    : nullptr;

            if (region) {
                auto surf = region->get_surface_collider(currentContact->collider.id);
                if (surf) {
                    Vec2f rpos = region->getPosition();
                    currentContact->collider = *surf;
                    currentContact->collider.surface.p1 += rpos;
                    currentContact->collider.surface.p2 += rpos;
                    currentContact->collider.ghostp0    += rpos;
                    currentContact->collider.ghostp3    += rpos;
                }
                else {
                    // surface no longer exists
                    currentContact = std::nullopt;
                }
            }
            else {
                // region no longer exists/contact is temporary
                currentContact = std::nullopt;
            }

			if (currentContact && !do_slope_wall_stop(colliders, had_wall)) {
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


Vec2f SurfaceTracker::calc_friction(Vec2f prevVel) const {
	Vec2f friction;
	if (has_contact() 
		&& (!currentContact->hasImpactTime || contact_time > 0.0)
		&& settings.has_friction) 
	{
        Vec2f tangent = math::projection(prevVel, currentContact->collider_n.righthand(), true);
        Vec2f normal = math::projection(prevVel, currentContact->collider_n, true);
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

CollidablePreMove SurfaceTracker::premove_update(poly_id_map<ColliderRegion>* colliders, secs deltaTime) {

	CollidablePreMove out;
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
		&& (math::dot(owner->get_local_vel(), currentContact->ortho_n) > 0);

	if (can_stop) {

		// correct velocity and position so we're still grounded
		owner->set_local_vel(Vec2f{});

		float X = owner->getPosition().x;
		Linef surface = currentContact->collider.surface;

		Vec2f intersect = math::intersection(
			surface,
			Linef{ Vec2f{X, 0.f}, Vec2f{X, 1.f} }
		);
		owner->setPosition(intersect, false);
	}
	return can_stop;

}

CollidablePreMove SurfaceTracker::do_move_with_platform(poly_id_map<ColliderRegion>* colliders, CollidablePreMove in) noexcept
{
	if (currentContact
        && currentContact->id
        && settings.move_with_platforms)
	{
		AppliedContact& contact = currentContact.value();
        const ColliderRegion* region = colliders->get(currentContact->id->collider);
        in.parent_vel = (region ? region->velocity : currentContact->velocity);

        /*
        in.surface_velocity = contact.surface_vel();
        */
	}
	return in;
}

CollidablePreMove SurfaceTracker::do_max_speed(CollidablePreMove in, secs deltaTime) noexcept {

	if (deltaTime > 0.0 
		&& has_contact() 
		&& settings.slope_sticking 
		&& settings.max_speed > 0.f) 
	{
		float speed   = *traverse_get_speed();
		Vec2f acc_vec = math::projection(owner->get_acc(), currentContact->collider_n.righthand(), true);
		float acc_mag = acc_vec.magnitude();

		if (owner->get_acc().x < 0.f) {
			acc_mag *= -1.f;
		}

		if (abs(speed + (acc_mag * deltaTime)) > settings.max_speed) {
			traverse_set_speed(settings.max_speed * (speed < 0.f ? -1.f : 1.f));
			in.acc_offset -= acc_vec;
		}
	}
	return in;
}

// ----------------------------

Vec2f SurfaceTracker::do_slope_stick(poly_id_map<ColliderRegion>* colliders, Vec2f wish_pos, Vec2f prev_pos) const
{
    if (!has_contact() || !currentContact->id)
        return {};

    struct visited_surface_t {
        ID<ColliderRegion> rid;
        ColliderSurfaceID sid;

        bool operator< (const visited_surface_t& rhs) const {
            if      (rid != rhs.rid)                 { return rid         < rhs.rid; }
            else if (sid.quad_id != rhs.sid.quad_id) { return sid.quad_id < rhs.sid.quad_id; }
            else                                     { return sid.dir     < rhs.sid.dir; }
        }
    };

    struct curr_surface_t {
        const ColliderRegion*  region  = nullptr;
        const ColliderSurface* surface = nullptr;
        Vec2f pos = {};
    };

    auto pick_best_surface = [&](const curr_surface_t& curr, const std::vector<touching_surface_t>& surfs) -> std::optional<touching_surface_t> {
        Vec2f curr_dir = math::vector(curr.surface->surface).unit();

        float curr_distsq = 0.f;
        const touching_surface_t* curr_pick = nullptr;

        for (auto& surf : surfs) {
            if (math::dot(curr_dir, curr.pos - surf.intersect) > 0) {
                // this intersect is behind us
                continue;
            }

            Vec2f normal = math::vector(surf.surface).lefthand().unit();
            float distsq = math::distSquared(surf.intersect, curr.pos);

            if (can_make_contact_with(normal)) {
                if (!curr_pick || curr_distsq > distsq) {
                    curr_pick = &surf;
                    curr_distsq = distsq;
                }
            }
        }

        return (curr_pick ? std::make_optional(*curr_pick) : std::nullopt);
    };

    curr_surface_t curr {
        .region  = colliders->get(currentContact->id->collider),
        .surface = &currentContact->collider,
        .pos     = prev_pos
    };

    std::set<visited_surface_t> visited = {
        { .rid = currentContact->id->collider, .sid = currentContact->collider.id }
    };
    std::vector<Vec2f> path {
        curr.pos
    };

    std::vector<touching_surface_t> touching_surfaces;

    float distance = math::dist(prev_pos, wish_pos);
    while (distance > 0.f) {
        touching_surfaces.clear();

        Linef surface = math::shift(curr.surface->surface, curr.region->getPosition());

        for (auto [id, ptr] : *colliders)
        {
            ptr->get_intersecting_surfaces(surface, std::back_inserter(touching_surfaces));
        }

        std::sort(
            touching_surfaces.begin(),
            touching_surfaces.end(),
            [](const touching_surface_t& lhs, const touching_surface_t& rhs) {
                return lhs.intersect.x < rhs.intersect.x;
            }
        );

        if (auto surf = pick_best_surface(curr, touching_surfaces)) {

        }
        else {
            return curr.pos + (distance * math::vector(curr.surface->surface).unit()) - wish_pos;
        }
    }

    return curr.pos - wish_pos;
}

CollidablePostMove SurfaceTracker::postmove_update(
        poly_id_map<ColliderRegion>* colliders,
        Vec2f wish_pos,
        Vec2f prev_pos
    ) const
{
	CollidablePostMove out;
	Vec2f position = wish_pos;

	if (settings.slope_sticking) {
        position += do_slope_stick(colliders, wish_pos, prev_pos);
	}

	out.pos_offset = position - wish_pos;
	return out;
}

void SurfaceTracker::start_touch(AppliedContact& contact) {

	if (settings.move_with_platforms) {
        owner->apply_parent_vel(contact.velocity);
        owner->apply_surface_vel(contact.surface_vel());
	}

	if (callbacks.on_start_touch)
		callbacks.on_start_touch(contact);
}

void SurfaceTracker::continue_touch(AppliedContact& contact) {
    if (settings.move_with_platforms) {
        owner->apply_surface_vel(contact.surface_vel());
        //owner->set_parent_vel(contact.velocity);
        //owner->set_last_parent_vel(contact.velocity);
    }
}

void SurfaceTracker::end_touch(AppliedContact& contact) {

	if (settings.move_with_platforms) {
        owner->reset_parent_vel();
        owner->reset_surface_vel();
	}

	if (callbacks.on_end_touch)
		callbacks.on_end_touch(contact);
}


void SurfaceTracker::traverse_set_speed(float speed) {
	if (has_contact()) 
	{
		Vec2f surf_unit = currentContact->collider_n.righthand();
        owner->set_local_vel(surf_unit * speed);
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

std::optional<float> SurfaceTracker::traverse_get_speed() const {
	std::optional<float> speed;
	if (has_contact()) {
		Vec2f surf = currentContact->collider_n.righthand();
		Vec2f proj = math::projection(owner->get_local_vel(), surf, true);

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
        currentContact = pc;

        //  project collidable velocity onto stickLine
		float vmag = owner->get_local_vel().magnitude();
        Vec2f vproj = math::projection(owner->get_local_vel(), math::vector(contact.stickLine)).unit();
        owner->set_local_vel(vmag * vproj);
        owner->setPosition(owner->getPosition() + (contact.ortho_n * contact.stickOffset), false);
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

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

    /*
    const ColliderRegion* region = nullptr;
    Vec2f region_offset;
    Vec2f region_delta;
    if (currentContact && currentContact->id) {
        region = colliders->get(currentContact->id->collider);
    }
    if (region) {
        region_offset = region->getPosition();
        region_delta  = region->getDeltaPosition();
    }

    //prev_pos += region_delta;
    */

    struct travel_surface_t {
        ID<ColliderRegion>     region_id;
        ColliderSurfaceID      surf_id;
        const ColliderRegion*  region  = nullptr;
        const ColliderSurface* surface = nullptr;
        Vec2f region_pos;
        Vec2f region_delta;
        Vec2f collider_n;
        Linef line;
        Vec2f pos;

        Angle vel_ang;
        float vel_mag;
    };

    auto make_travel_surface = [&colliders](ID<ColliderRegion> region_id, ColliderSurfaceID surf_id, const ColliderSurface* surf, Vec2f travel_start_pos, Vec2f collider_n) {
        travel_surface_t tmp{
            .region_id    = region_id,
            .surf_id      = surf_id,
            .pos          = travel_start_pos
        };
        tmp.region          = colliders->get(region_id);
        tmp.surface         = tmp.region->get_surface_collider(surf_id);
        tmp.region_pos      = tmp.region->getPosition();
        tmp.region_delta    = tmp.region->getDeltaPosition();
        tmp.line            = math::shift(surf->surface, tmp.region_delta);
        //tmp.line            = surf->surface;
        tmp.collider_n      = collider_n;

        return tmp;
    };

    travel_surface_t curr = make_travel_surface(currentContact->id->collider, currentContact->collider.id, &currentContact->collider, prev_pos, currentContact->collider_n);

    std::vector<Vec2f> path {
        curr.pos
    };

    float travel_dir = 0.f;
    {
        Vec2f curr_vec = math::vector(curr.line);
        float dir_dot  = math::dot(math::projection(wish_pos - prev_pos, curr_vec), curr_vec);
        if (dir_dot > 0.f) {
            travel_dir = 1.f;
        }
        else if (dir_dot < 0.f) {
            travel_dir = -1.f;
        }
    }

    // moving orthogonal to surface
    //if (travel_dir == 0.f)
    //    return {};

    std::vector<travel_surface_t> touching_surfaces;

    auto valid_surface =
        [&]
        (ID<ColliderRegion> region_id, ColliderSurfaceID surf_id, const ColliderRegion* region, const ColliderSurface* surf, Rectf bounds, Linef surface) -> std::optional<travel_surface_t>
    {
        if (!surf) {
            //LOG_INFO("NO SURF");
            return {};
        }

        if (region_id == curr.region_id && surf_id == curr.surf_id) {
            return {};
        }

        Linef msurf = math::shift(surf->surface, region->getPosition());
        Vec2f inter = math::intersection(msurf, surface);

        bool inter_in_bounds = bounds.contains(inter);
        bool is_collinear = math::collinear(msurf, surface);
        auto msurf_bounds = math::line_bounds(msurf);
        bool line_bounds = bounds.touches(msurf_bounds);

        std::optional<Vec2f> intersect;
        if (inter != Vec2f{NAN, NAN} && bounds.contains(inter))
        {
            intersect = inter;
        }
        else if (math::collinear(msurf, surface)) {
            if (bounds.touches(math::line_bounds(msurf))) {
                intersect = (travel_dir > 0.f ? surface.p2 : surface.p1);
            }
            else if (bounds.height == 0.f
                && (msurf_bounds.left <= bounds.left + bounds.width
                    || bounds.left <= msurf_bounds.left + msurf_bounds.width))
            {
                intersect = (travel_dir > 0.f ? surface.p2 : surface.p1);
            }
            else if (bounds.width == 0.f
                    && (msurf_bounds.top <= bounds.top + bounds.height
                        || bounds.top <= msurf_bounds.top + msurf_bounds.height))
            {
                intersect = (travel_dir > 0.f ? surface.p2 : surface.p1);
            }
        }

        if (!intersect) {
            LOG_INFO("NO INTERSECT");
            return {};
        }

        Angle next_ang = math::angle(math::tangent(surf->surface));
        Angle curr_ang = math::angle(curr.collider_n.righthand());
        Angle diff = next_ang - curr_ang;

        LOG_INFO("[{}] {} -> {} : {}", travel_dir, curr_ang.degrees(), next_ang.degrees(), diff.degrees());
        LOG_INFO("{} && {}", angle_range.within_range(next_ang - Angle::Degree(90.f)), abs(diff.degrees()) < abs(settings.stick_angle_max.degrees()));

        if (angle_range.within_range(next_ang - Angle::Degree(90.f))
            && abs(diff.degrees()) < abs(settings.stick_angle_max.degrees()))
        {

            LOG_INFO("IN");
            //Vec2f hyp = wish_pos - ((travel_dir > 0.f ? surf->surface.p2 : surf->surface.p1) + region->getPosition() + region->getDeltaPosition());
            //Angle theta = math::angle(hyp) - math::angle(surf->surface);

            // update velocity
            Angle gAng = math::angle(msurf);
            if (travel_dir < 0.f) {
                gAng -= Angle::Degree(180.f);
            }

            float slow = 1.f - settings.slope_stick_speed_factor * abs(diff.degrees() / settings.stick_angle_max.degrees());

            Angle vang = gAng;
            float vmag = (owner->get_local_vel() + owner->get_surface_vel()).magnitude() * slow;

            //if (theta.degrees() < 0.f && abs(diff.degrees()) < abs(settings.stick_angle_max.degrees())) {

                // update position
                //float dist = hyp.magnitude() * sin(theta.radians());
                //Vec2f offset = math::vector(surf->surface).lefthand().unit();

                //if (callbacks.on_stick)
                //    callbacks.on_stick(*surf);

                //return offset * dist;

                travel_surface_t travel;

                travel.region_id;
                travel.surf_id;
                travel.region  = region;
                travel.surface = surf;
                travel.region_pos = region->getPosition();
                travel.region_delta = region->getDeltaPosition();
                travel.collider_n = math::normal(msurf);
                travel.line     = msurf;
                travel.pos      = *intersect;
                travel.vel_ang  = vang;
                travel.vel_mag  = vmag;

                return travel;
            //}
        }

        return {};
    };

    auto get_touching_surfaces =
        [&]
        (ID<ColliderRegion> region_id, const ColliderRegion* region, Linef surface)
    {
        size_t count = 0;
        Rectf bounds = math::line_bounds(surface);
        for (auto& quad : region->in_rect(bounds)) {
            for (auto dir: direction::cardinals) {
                auto* surf = quad.getSurface(dir);
                ColliderSurfaceID surf_id = { quad.getID(), dir };
                if (auto travel_surf = valid_surface(region_id, surf_id, region, surf, bounds, surface)) {

                    travel_surf->region_id = region_id;
                    travel_surf->surf_id = surf_id;

                    touching_surfaces.emplace_back(*travel_surf);
                    ++count;
                }
            }
        }
        if (count > 0) {
            LOG_INFO("QUADS: {}", count);
        }
    };

    auto surface_cmp =
        [&]
        (const travel_surface_t& curr_pick, const travel_surface_t& candidate) -> bool
    {
        float curr_dist = math::dist(curr_pick.pos, curr.pos);
        float cand_dist = math::dist(candidate.pos, curr.pos);

        if (cand_dist < curr_dist) {
            return true;
        }

        Angle ang       = math::angle(curr.line);
        Angle curr_ang  = math::angle(curr_pick.line) - ang;
        Angle cand_ang  = math::angle(candidate.line) - ang;

        if (travel_dir > 0.f  ? cand_ang > curr_ang : cand_ang < curr_ang) {
            return true;
        }

        return false;
    };

    auto pick_best_surface =
        [this, &surface_cmp, &travel_dir]
        (const travel_surface_t& curr, const std::vector<travel_surface_t>& candidates) -> std::optional<travel_surface_t>
    {
        Vec2f curr_dir = travel_dir * math::vector(curr.line);

        const travel_surface_t* curr_pick = nullptr;

        for (auto& candidate : candidates) {
            // is the same surface
            bool is_same = candidate.region_id == curr.region_id && candidate.surf_id == curr.surf_id;

            // surface intersect is on or behind us
            float dir_dot = math::dot(curr_dir, candidate.pos - curr.pos);

            if (is_same || (dir_dot <= 0)) {
                continue;
            }

            Vec2f normal = math::vector(candidate.line).lefthand().unit();

            if (can_make_contact_with(normal)) {
                if (!curr_pick || surface_cmp(curr, candidate)) {
                    curr_pick   = &candidate;
                }
            }
        }

        return (curr_pick ? std::make_optional(*curr_pick) : std::nullopt);
    };

    auto travel_to = [&](const travel_surface_t& from, const travel_surface_t& to, float remaining_dist) -> float {
        Vec2f unit = (to.pos - from.pos).unit();
        float dist = math::dist(from.pos, to.pos);

        Vec2f npos;
        if (dist > remaining_dist) {
            npos = from.pos + (remaining_dist * unit);
            remaining_dist = 0.f;
        }
        else {
            LOG_INFO("[{}, {}] SWITCH TO {} -> {} AT {}", remaining_dist, travel_dir, to.line.p1, to.line.p2, to.pos);
            npos = to.pos;
            remaining_dist -= dist;

            // DO VELOCITY UPDATE SOMEWHERE
            owner->reset_surface_vel();
            //float vel_mag = owner->get_local_vel().magnitude() * slow;
            owner->set_local_vel(Vec2f{
                    cosf(to.vel_ang.radians()),
                    sinf(to.vel_ang.radians())
            } * to.vel_mag);

            if (callbacks.on_stick)
                callbacks.on_stick(*to.surface);
        }
        path.push_back(npos);

        return remaining_dist;
    };

    //float distance = math::dist(prev_pos, prev_pos + math::projection(wish_pos - prev_pos, math::vector(curr.line)));
    float distance = math::dist(prev_pos, wish_pos);

    if (distance == 0.f)
        return {};

    while (distance > 0.f) {
        touching_surfaces.clear();

        for (auto [collider_id, ptr] : *colliders) {
            get_touching_surfaces(collider_id, ptr.get(), curr.line);
        }

        if (auto surf = pick_best_surface(curr, touching_surfaces)) {
            distance = travel_to(curr, *surf, distance);
            curr = *surf;
            curr.pos = path.back();
        }
        else {
            break;
        }
    }

    size_t i = 0;
    for (auto p : path) {
        LOG_INFO("PATH[{}]: {}", i, p);
        ++i;
    }

    if (distance == 0.f) {
        return curr.pos - wish_pos;
    }
    else {
        return (curr.pos + ( distance * travel_dir * math::vector(curr.line).unit() )) - wish_pos;
    }
}

CollidablePostMove SurfaceTracker::postmove_update(
        poly_id_map<ColliderRegion>* colliders,
        Vec2f wish_pos,
        Vec2f prev_pos
    ) const
{

    if (has_contact() && currentContact->id->collider) {
        if (auto region = colliders->get(currentContact->id->collider)) {
            prev_pos += region->getDeltaPosition();
        }
    }


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

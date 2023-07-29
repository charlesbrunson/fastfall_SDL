#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game/phys/ColliderRegion.hpp"
#include "fastfall/game/phys/collidable/SurfaceFollow.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"

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
    if (!has_contact() || !currentContact->id) {
        return {};
    }

    auto init_region_id = currentContact->id->collider;
    auto init_surf_id   = currentContact->collider.id;
    auto init_region    = colliders->get(init_region_id);
    auto init_surface   = init_region ? init_region->get_surface_collider(init_surf_id) : nullptr;

    if (!init_surface) {
        return {};
    }

    Linef init_path = math::shift(init_surface->surface, init_region->getPosition());
    Vec2f init_pos  = prev_pos;

    float travel_dir = 0.f;
    {
        Vec2f curr_vec = math::vector(init_path);
        float dir_dot  = math::dot(math::projection(wish_pos - prev_pos, curr_vec), curr_vec);
        if (dir_dot > 0.f) {
            travel_dir = 1.f;
        }
        else if (dir_dot < 0.f) {
            travel_dir = -1.f;
        }
    }

    float distance = math::dist(prev_pos, wish_pos);

    if (distance == 0.f) {
        return {};
    }

    struct surface_id {
        ID<ColliderRegion> region_id;
        ColliderSurfaceID  surf_id;
    };

    std::map<SurfaceFollow::surface_id, surface_id> surface_map;
    SurfaceFollow follower { init_path, init_pos, travel_dir, distance, angle_range, settings.stick_angle_max };

    std::vector<Rectf> quads_visited;

    Vec2f curr_pos = init_pos;
    //LOG_INFO("path follow");
    size_t interations = 0;
    while (follower.remaining_distance() > 0.f) {
        //LOG_INFO("iter {} at {}->{}", interations++, follower.current_path().line.p1, follower.current_path().line.p2);
        surface_map.clear();

        Rectf bounds = math::line_bounds(follower.current_path().line);

        size_t tried = 0;
        for (auto [region_id, region_ptr] : *colliders) {

            for (auto& quad : region_ptr->in_rect(bounds)) {
                if (!quad.hasAnySurface())
                    continue;

                if (auto quad_bounds = quad.get_bounds())
                    quads_visited.push_back(math::shift(*quad_bounds, region_ptr->getPosition()));

                for (auto dir: direction::cardinals) {
                    if (auto* surf = quad.getSurface(dir)) {

                        ColliderSurfaceID surf_id = { quad.getID(), dir };
                        Linef line = math::shift(surf->surface, region_ptr->getPosition());
                        ++tried;
                        bool good = false;
                        if (auto id = follower.add_surface(line)) {
                            good = true;
                            surface_map.emplace(*id, surface_id{ region_id, surf_id });
                        }

                        /*
                        if (auto* tilemap = dynamic_cast<ColliderTileMap*>(region_ptr.get())) {
                            LOG_INFO("\t[{}] candidate {}: {}->{}", (good ? "good" : "bad "), tilemap->to_pos(quad.getID()), line.p1, line.p2 );
                        }
                        else {
                            LOG_INFO("\t[{}] candidate {}: {}->{}", (good ? "good" : "bad "), quad.getID().value, line.p1, line.p2 );
                        }
                        */

                    }
                }
            }
        }

        if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_TRACKER)) {
            auto& lines = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_TRACKER>(Primitive::TRIANGLES, follower.get_path_candidates().size() * 6);
            size_t n = 0;
            for (auto& can : follower.get_path_candidates()) {
                lines[(n * 6) + 0].pos = can.line.p1;
                lines[(n * 6) + 1].pos = can.line.p2;
                lines[(n * 6) + 2].pos = can.line.p1 - math::normal(can.line) * 2.f;
                lines[(n * 6) + 3].pos = can.line.p1 - math::normal(can.line) * 2.f;
                lines[(n * 6) + 4].pos = can.line.p2;
                lines[(n * 6) + 5].pos = can.line.p2 - math::normal(can.line) * 2.f;

                lines[(n * 6) + 0].color = ff::Color::Red;
                lines[(n * 6) + 1].color = ff::Color::Red;
                lines[(n * 6) + 2].color = ff::Color::Red;
                lines[(n * 6) + 3].color = ff::Color::Red;
                lines[(n * 6) + 4].color = ff::Color::Red;
                lines[(n * 6) + 5].color = ff::Color::Red;
                ++n;
            }

        }

        if (auto id = follower.pick_surface_to_follow()) {
            auto result = follower.travel_to(*id);

            curr_pos = result.pos;
            //LOG_INFO("\ttravel to {}->{} at {}", result.path.line.p1, result.path.line.p2, curr_pos);

            if (result.on_new_surface) {
                float slow = 1.f - settings.slope_stick_speed_factor *
                                   abs(result.path.diff_angle.degrees() / settings.stick_angle_max.degrees());

                owner->reset_surface_vel();

                float vmag = owner->get_local_vel().magnitude() * slow;

                owner->set_local_vel(Vec2f{
                        cosf(result.path.angle.radians()),
                        sinf(result.path.angle.radians())
                } * vmag);

                if (callbacks.on_stick) {
                    auto surface = surface_map.at(result.path.id);
                    if (auto region = colliders->get(surface.region_id)) {
                        if (auto surf = region->get_surface_collider(surface.surf_id)) {
                            callbacks.on_stick(*surf);
                        }
                    }
                }
            }
        }
        else {
            //LOG_INFO("\ttravel break");
            auto result = follower.finish();
            curr_pos = result.pos;
            break;
        }
    }


    if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_TRACKER)) {

        auto& quad_lines = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_TRACKER>(Primitive::LINES, quads_visited.size() * 8);
        size_t n = 0;
        for (auto& bounds : quads_visited) {
            quad_lines[(n * 8) + 0].pos = bounds.topleft();
            quad_lines[(n * 8) + 1].pos = bounds.topright();
            quad_lines[(n * 8) + 2].pos = bounds.topright();
            quad_lines[(n * 8) + 3].pos = bounds.botright();
            quad_lines[(n * 8) + 4].pos = bounds.botright();
            quad_lines[(n * 8) + 5].pos = bounds.botleft();
            quad_lines[(n * 8) + 6].pos = bounds.botleft();
            quad_lines[(n * 8) + 7].pos = bounds.topleft();

            for (int i = 0; i < 8; ++i) {
                quad_lines[(n * 8) + i].color = ff::Color::White;
            }

            ++n;
        }

        auto& lines = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_TRACKER>(Primitive::TRIANGLES, follower.get_path_taken().size() * 6);
        n = 0;
        for (auto& can : follower.get_path_taken()) {
            lines[(n * 6) + 0].pos = can.line.p1;
            lines[(n * 6) + 1].pos = can.line.p2;
            lines[(n * 6) + 2].pos = can.line.p1 - math::normal(can.line) * 2.f;
            lines[(n * 6) + 3].pos = can.line.p1 - math::normal(can.line) * 2.f;
            lines[(n * 6) + 4].pos = can.line.p2;
            lines[(n * 6) + 5].pos = can.line.p2 - math::normal(can.line) * 2.f;

            lines[(n * 6) + 0].color = ff::Color::Green;
            lines[(n * 6) + 1].color = ff::Color::Green;
            lines[(n * 6) + 2].color = ff::Color::Green;
            lines[(n * 6) + 3].color = ff::Color::Green;
            lines[(n * 6) + 4].color = ff::Color::Green;
            lines[(n * 6) + 5].color = ff::Color::Green;
            ++n;
        }
    }

    return curr_pos - wish_pos;
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

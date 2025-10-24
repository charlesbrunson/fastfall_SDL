#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/game/phys/collision/Contact.hpp"

#include "fastfall/util/id_map.hpp"
#include "SurfaceFollow.hpp"

#include <optional>
#include <functional>

namespace ff {

class Collidable;

struct Friction {
	float stationary = 0.f;
	float kinetic = 0.f;
};

struct CollidablePreMove {
    Vec2f pos_offset;
    Vec2f vel_offset;
    Vec2f acc_offset;
    Vec2f parent_vel;
};

struct CollidablePostMove {
    Vec2f pos_offset;
};

// track contact duration for surfaces between the given angle ranges
class SurfaceTracker {
public:
	SurfaceTracker(Collidable* t_owner, Angle ang_min, Angle ang_max, bool inclusive = true);

	CollidablePreMove  premove_update(poly_id_map<ColliderRegion>* colliders, secs deltaTime);
	CollidablePostMove postmove_update(poly_id_map<ColliderRegion>* colliders, Vec2f wish_pos, Vec2f prev_pos) const;

    void update_collidable_ptr(Collidable* t_owner) { owner = t_owner; }

	void process_contacts(
            poly_id_map<ColliderRegion>* colliders,
            std::vector<AppliedContact>& contacts);

	[[nodiscard]] bool has_contact() const noexcept;
    [[nodiscard]] bool has_contact_with(ID<ColliderRegion> collider) const noexcept;

	// applies velocity/acceleration accounting for gravity (for movement consistency)
	// if !has_contact, no op
	[[nodiscard]] std::optional<float> traverse_get_speed() const;
	void traverse_set_speed(float speed);
	void traverse_add_accel(float accel);
	void traverse_add_decel(float decel);

    void force_end_contact();

	[[nodiscard]] bool can_make_contact_with(const AppliedContact& contact) const noexcept;
    [[nodiscard]] bool can_make_contact_with(Vec2f collier_normal) const noexcept;

	[[nodiscard]] const std::optional<AppliedContact>& get_contact() const { return currentContact; };


	void firstCollisionWith(const AppliedContact& contact);

	// time in contact
	// this will propagate across different surfaces
	// as long as they're within the angle range
	secs contact_time = 0.0;
	secs air_time = 0.0;

	struct callbacks_t {
		std::function<void(Collidable&, AppliedContact&)> on_start_touch;
		std::function<void(Collidable&, AppliedContact&)> on_end_touch;
		std::function<void(Collidable&, const SurfaceFollow::travel_result&, const ColliderSurface& surface)> on_stick;
	} callbacks;

	struct Settings {
		bool move_with_platforms = false;
		bool slope_sticking = false;
		bool slope_wall_stop = false;
		bool has_friction = false;
		bool use_surf_vel = false;

		Angle stick_angle_max;
		Friction surface_friction;
		float max_speed = 0.f;

		float slope_stick_speed_factor = 0.25f;
	} settings;

	AngleRange angle_range;

    [[nodiscard]] Vec2f calc_friction(Vec2f prevVel) const;
    float accel_accum = 0.f;

private:

	// the best suited contact for this recorder
	// from the current contact frame
	// based on angle and contact duration
	std::optional<AppliedContact> currentContact = std::nullopt;
	std::optional<AppliedContact> wallContact = std::nullopt;

	bool do_slope_wall_stop(poly_id_map<ColliderRegion>* colliders, bool had_wall) noexcept;
	CollidablePreMove do_move_with_platform(poly_id_map<ColliderRegion>* colliders, CollidablePreMove in) noexcept;
	CollidablePreMove do_max_speed(CollidablePreMove in, secs deltaTime) noexcept;

    void start_touch(AppliedContact& contact);
    void continue_touch(AppliedContact& contact);
    void end_touch(AppliedContact& contact);

	// returns position offset
	Vec2f do_slope_stick(poly_id_map<ColliderRegion>* colliders, Vec2f wish_pos, Vec2f prev_pos) const;

    Collidable* owner;
};

}

#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/game_v2/phys/collision/Contact.hpp"

#include "fastfall/util/id_map.hpp"

#include <optional>
#include <functional>

namespace ff {

class Collidable;

struct Friction {
	float stationary = 0.f;
	float kinetic = 0.f;
};

struct CollidableOffsets {
	Vec2f position = { 0.f, 0.f };
	Vec2f velocity = { 0.f, 0.f };
	Vec2f acceleration = { 0.f, 0.f };
};

// track contact duration for surfaces between the given angle ranges
class SurfaceTracker {
public:
	SurfaceTracker(ID<Collidable> t_owner, Angle ang_min, Angle ang_max, bool inclusive = true);

	CollidableOffsets premove_update(secs deltaTime);
	CollidableOffsets postmove_update(Vec2f wish_pos, Vec2f prev_pos, std::optional<PersistantContact> contact_override = {}) const;

	void process_contacts(Collidable* owner, std::vector<PersistantContact>& contacts);
	bool has_contact() const noexcept;

	// applies velocity/acceleration accounting for gravity (for movement consistency)
	// if !has_contact, no op
	std::optional<float> traverse_get_speed();
	void traverse_set_speed(float speed);
	void traverse_add_accel(float accel);
	void traverse_add_decel(float decel);

	inline ID<Collidable> get_collidable_id() { return owner_id; };
    inline void set_collidable_ptr(Collidable* ptr) { owner = ptr;}
    inline void set_colliders_ptr(poly_id_map<ColliderRegion>* ptr) { colliders = ptr;}

	bool can_make_contact_with(const Contact& contact) const noexcept;

	const std::optional<PersistantContact>& get_contact() { return currentContact; };

	void start_touch(PersistantContact& contact);
	void end_touch(PersistantContact& contact);

	void firstCollisionWith(const Contact& contact);

	// time in contact
	// this will propogate across different surfaces
	// as long as they're within the angle range
	secs contact_time = 0.0;
	secs air_time = 0.0;

	struct callbacks_t {
		std::function<void(PersistantContact&)> on_start_touch;
		std::function<void(PersistantContact&)> on_end_touch;
		std::function<void(const ColliderSurface&)> on_stick;
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

	struct applicable_ang_t {
		Angle min;
		Angle max;
		bool  inclusive;

		inline bool within_range(Angle ang) const { 
			return ang.isBetween(min, max, inclusive); 
		};
		inline void set_angle_range(Angle ang_min, Angle ang_max, bool inclusive = true) { 
			min = ang_min; 
			max = ang_max; 
			inclusive = inclusive;
		};
	} angle_range;

private:
	// the best suited contact for this recorder
	// from the current contact frame
	// based on angle and contact duration
	std::optional<PersistantContact> currentContact = std::nullopt;
	std::optional<PersistantContact> wallContact = std::nullopt;

	bool do_slope_wall_stop(bool had_wall) noexcept;
	CollidableOffsets do_move_with_platform(CollidableOffsets in) noexcept;
	CollidableOffsets do_max_speed(CollidableOffsets in, secs deltaTime) noexcept;

	// returns position offset
	Vec2f do_slope_stick(Vec2f wish_pos, Vec2f prev_pos, float left, float right, const PersistantContact& contact) const noexcept;

	Vec2f calc_friction(Vec2f prevVel);

    ID<Collidable> owner_id;
    Collidable* owner;
    poly_id_map<ColliderRegion>* colliders;
};

}

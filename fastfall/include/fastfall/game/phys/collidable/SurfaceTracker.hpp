#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/game/phys/Identity.hpp"
#include "fastfall/game/phys/collision/Contact.hpp"
#include "fastfall/game/GameContext.hpp"

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
	SurfaceTracker(GameContext game_context, Angle ang_min, Angle ang_max, bool inclusive = true);

	// acceleration out
	CollidableOffsets premove_update(secs deltaTime);

	// new position out
	CollidableOffsets postmove_update(Vec2f wish_pos, Vec2f prev_pos, secs deltaTime);

	void process_contacts(std::vector<PersistantContact>& contacts);
	bool has_contact() const noexcept;

	// applies velocity/acceleration accounting for gravity (for movement consistency)
	// is !has_contact, will apply on the X axis
	//void traverse_set_max_speed(float speed);
	//float traverse_get_max_speed() const noexcept { return max_speed; };
	void traverse_set_speed(float speed);
	void traverse_add_accel(float accel);
	void traverse_add_decel(float decel);

	float traverse_get_speed();

	inline Collidable* get_collidable() { return owner; };

	inline void time_reset() { contact_time = 0.0; air_time = 0.0; };

	inline void contact_time_acc(secs deltaTime) { contact_time += deltaTime; };
	inline secs get_contact_time() const noexcept { return contact_time; };

	inline void air_time_acc(secs deltaTime) { air_time += deltaTime; };
	inline secs get_air_time() const noexcept { return air_time; };

	inline bool is_angle_in_range(Angle ang) { return ang.isBetween(angle_min, angle_max, angle_inclusive); }
	inline Angle get_angle_min() { return angle_min; };
	inline Angle get_angle_max() { return angle_max; };
	inline bool get_angle_inclusive() { return angle_inclusive; };

	inline void set_angle_range(Angle ang_min, Angle ang_max, bool inclusive = true) { 
		angle_min = ang_min; 
		angle_max = ang_max; 
		angle_inclusive = inclusive;
	};

	const std::optional<PersistantContact>& get_contact() { return currentContact; };

	void start_touch(PersistantContact& contact);
	void end_touch(PersistantContact& contact);

	inline void set_start_touch(std::function<void(PersistantContact&)> func) {
		callback_start_touch = func;
	}
	inline void set_end_touch(std::function<void(PersistantContact&)> func) {
		callback_end_touch = func;
	}
	inline void set_on_stick(std::function<void(const ColliderSurface&)> func) {
		callback_on_stick = func;
	}

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

private:
	friend class Collidable;

	// the best suited contact for this recorder
	// from the current contact frame
	// based on angle and contact duration
	std::optional<PersistantContact> currentContact = std::nullopt;

	bool do_slope_wall_stop(bool had_wall) noexcept;
	CollidableOffsets do_move_with_platform(CollidableOffsets in) noexcept;
	Vec2f do_max_speed(secs deltaTime) noexcept;

	float wallYadj = 0.f;

	// applied when max_speed > 0.f

	std::function<void(PersistantContact&)> callback_start_touch;
	std::function<void(PersistantContact&)> callback_end_touch;
	std::function<void(const ColliderSurface&)> callback_on_stick;

	std::optional<PersistantContact> wallContact;

	GameContext context;

	Vec2f get_friction(Vec2f prevVel);

	// time in contact
	// this will propogate across different surfaces
	// as long as they're within the angle range
	secs contact_time = 0.0;
	secs air_time = 0.0;

	// applicable range
	// based on contact surface_normal
	Angle angle_min;
	Angle angle_max;
	bool angle_inclusive;

	Collidable* owner = nullptr;
};

}
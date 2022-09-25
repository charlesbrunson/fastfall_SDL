#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/engine/time/time.hpp"

#include "fastfall/game/phys/collision/Contact.hpp"
#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

#include <assert.h>
#include <vector>
#include <optional>
#include <memory>

namespace ff {

class World;

struct collision_state_t {
	enum class flags : unsigned {
		None = 0,
		Floor = 1,
		Wall_L,
		Wall_R,
		Ceiling,

		Crush_H,
		Crush_V,
		Wedge,
	};

	void reset() { flags = 0u; }
	void set_flag(flags type) { flags |= (1 << static_cast<unsigned>(type)); }

	bool has_set(flags type) const { return flags & (1 << static_cast<unsigned>(type)); }

	template<class ... Flags>
	bool has_any(Flags&&... flag) const noexcept {
		return (has_set(flag) || ...);
	};

	template<class ... Flags>
	bool has_all(Flags&&... flag) const noexcept {
		return (has_set(flag) && ...);
	};

	unsigned value() const { return flags; }

private:
	unsigned flags = 0u;
};

class Collidable {
public:
	enum class SlipState {
		SlipHorizontal,
		SlipVertical
	};

	struct slip_t {
		SlipState state = SlipState::SlipHorizontal;
		float leeway = 0.f;
	};

public:
	Collidable(Vec2f position, Vec2f size, Vec2f gravity = Vec2f{});

    Collidable(const Collidable&);
    Collidable(Collidable&&) noexcept;

    Collidable& operator=(const Collidable&);
    Collidable& operator=(Collidable&&) noexcept;

	void update(poly_id_map<ColliderRegion>* colliders, secs deltaTime);

	Rectf getBoundingBox();

	inline Rectf getBox() const noexcept { return curRect; };
	inline Rectf getPrevBox() const noexcept { return prevRect; };

	inline Vec2f getPosition() const noexcept { return pos; };
	inline Vec2f getPrevPosition() const noexcept { return prevPos; };

	void setPosition(Vec2f position, bool swapPrev = true) noexcept;

	inline void move(Vec2f offset, bool swapPrev = true) {
		setPosition(getPosition() + offset, swapPrev);
	};

	void setSize(Vec2f size) noexcept;
	void teleport(Vec2f position) noexcept;

	inline Vec2f get_gravity() const noexcept { return gravity_acc; };
	inline void set_gravity(Vec2f grav) noexcept { gravity_acc = grav; };

	inline Vec2f get_vel()   const noexcept { return vel; };

	inline void  set_vel(Vec2f velocity) noexcept { vel = velocity; };
	inline void  set_vel(std::optional<float> X, std::optional<float> Y) noexcept { 
		if (X) { vel.x = *X; }
		if (Y) { vel.y = *Y; }
	}

	inline void add_accel(Vec2f acceleration) { accel_accum += acceleration; };
	inline void add_decel(Vec2f deceleration) { decel_accum += deceleration; };

	inline Vec2f get_friction()   const noexcept { return friction; };
	inline Vec2f get_acc() const noexcept { return acc; };

	void applyContact(const AppliedContact& contact, ContactType type);

	void debug_draw() const;

	const AppliedContact* get_contact(Angle angle) const noexcept;
	const AppliedContact* get_contact(Cardinal dir) const noexcept;

	bool has_contact(Angle angle) const noexcept;
	bool has_contact(Cardinal dir) const noexcept;

	inline const std::vector<AppliedContact>& get_contacts() const noexcept { return currContacts; };

	void set_frame(poly_id_map<ColliderRegion>* colliders, std::vector<AppliedContact>&& frame);

	void setSlip(slip_t set) { slip = set; };

	bool    hasSlip()   const noexcept { return slip.leeway != 0.f; };
	bool    hasSlipH()  const noexcept { return hasSlip() && slip.state == SlipState::SlipHorizontal; };
	bool    hasSlipV()  const noexcept { return hasSlip() && slip.state == SlipState::SlipVertical; };
	slip_t  getSlip()   const noexcept { return slip; }


	const collision_state_t& get_state_flags() const { return col_state; }

    std::pair<ID<SurfaceTracker>, SurfaceTracker*>
    create_tracker(Angle ang_min, Angle ang_max, bool inclusive = true);

    bool erase_tracker(ID<SurfaceTracker> id);

    SurfaceTracker* get_tracker(ID<SurfaceTracker> id) { return trackers.get(id); };
    const SurfaceTracker* get_tracker(ID<SurfaceTracker> id) const { return trackers.get(id); };

    id_map<SurfaceTracker>& get_trackers() { return trackers; };
    const id_map<SurfaceTracker>& get_trackers() const { return trackers; };

    struct callbacks_t {
        std::function<void(World&)> onPostCollision;
    } callbacks;

private:

	collision_state_t col_state;

	slip_t slip;

	Vec2f pos;
	Vec2f prevPos;

	Rectf curRect;
	Rectf prevRect;

	Vec2f vel;
	Vec2f precollision_vel; // velocity saved before collision, used for friction calculation

	Vec2f friction;

	Vec2f acc;
	Vec2f gravity_acc;
	Vec2f accel_accum;
	Vec2f decel_accum;

	std::vector<AppliedContact> currContacts;
    id_map<SurfaceTracker> trackers;
};

}

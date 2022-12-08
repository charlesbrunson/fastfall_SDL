#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/engine/time/time.hpp"

#include "fastfall/game/phys/collision/Contact.hpp"
#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"
#include "fastfall/game/attach/AttachPoint.hpp"
#include "fastfall/util/id.hpp"

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

	inline Rectf getBox() const noexcept { return currRect; };
	inline Rectf getPrevBox() const noexcept { return prevRect; };

	inline Vec2f getPosition() const noexcept { return currPos; };
	inline Vec2f getPrevPosition() const noexcept { return prevPos; };

	void setPosition(Vec2f position, bool swapPrev = true) noexcept;

	inline void move(Vec2f offset, bool swapPrev = true) {
		setPosition(getPosition() + offset, swapPrev);
	};

	void setSize(Vec2f size) noexcept;
	void teleport(Vec2f position) noexcept;

	Vec2f get_gravity() const noexcept;
	void  set_gravity(Vec2f grav) noexcept;

	Vec2f get_local_vel()   const noexcept;
    Vec2f get_parent_vel()  const noexcept;
    Vec2f get_surface_vel() const noexcept;
    Vec2f get_global_vel()  const noexcept;

	void set_local_vel(Vec2f velocity) noexcept;
	void set_local_vel(std::optional<float> X, std::optional<float> Y) noexcept;
    void set_parent_vel(Vec2f pvel) noexcept;
    void set_surface_vel(Vec2f svel) noexcept;

    Vec2f get_last_parent_vel() const noexcept;
    void set_last_parent_vel(Vec2f pvel) noexcept;

	void add_accel(Vec2f acceleration);
	void add_decel(Vec2f deceleration);

	Vec2f get_friction() const noexcept;
	Vec2f get_acc() const noexcept;

	void applyContact(const AppliedContact& contact, ContactType type);

	void debug_draw() const;

	const AppliedContact* get_contact(Angle angle) const noexcept;
	const AppliedContact* get_contact(Cardinal dir) const noexcept;

	bool has_contact(Angle angle) const noexcept;
	bool has_contact(Cardinal dir) const noexcept;

	inline const std::vector<AppliedContact>& get_contacts() const noexcept { return currContacts; };

	void set_frame(poly_id_map<ColliderRegion>* colliders, std::vector<AppliedContact>&& frame);

	void    setSlip(slip_t set) noexcept { slip = set; };
	bool    hasSlip()   const noexcept { return slip.leeway != 0.f; };
	bool    hasSlipH()  const noexcept { return hasSlip() && slip.state == SlipState::SlipHorizontal; };
	bool    hasSlipV()  const noexcept { return hasSlip() && slip.state == SlipState::SlipVertical; };
	slip_t  getSlip()   const noexcept { return slip; }


	const collision_state_t& get_state_flags() const { return col_state; }

    std::optional<SurfaceTracker>& tracker() { return _tracker; }
    const std::optional<SurfaceTracker>& tracker() const { return _tracker; }
    void create_tracker(Angle ang_min, Angle ang_max, bool inclusive = true);
    bool erase_tracker();


    struct callbacks_t {
        std::function<void(World&)> onPostCollision;
    } callbacks;

    void set_attach_id(ID<AttachPoint> id) { attachpoint = id; }
    ID<AttachPoint> get_attach_id() const { return attachpoint; }
    void  set_attach_origin(Vec2f offset) { attachpoint_origin = offset; }
    Vec2f get_attach_origin() const { return attachpoint_origin; }

private:

    ID<AttachPoint> attachpoint;
    Vec2f attachpoint_origin;

	collision_state_t col_state;

	slip_t slip;

	Vec2f currPos;
	Vec2f prevPos;

	Rectf currRect;
	Rectf prevRect;

    Vec2f last_parent_vel;
    Vec2f parent_vel;
    Vec2f surface_vel;
	Vec2f local_vel;
	Vec2f local_precollision_vel; // velocity saved before collision, used for friction calculation

	Vec2f friction;

	Vec2f acc;
	Vec2f gravity_acc;
	Vec2f accel_accum;
	Vec2f decel_accum;

    std::optional<SurfaceTracker> _tracker;
	std::vector<AppliedContact> currContacts;
};

}

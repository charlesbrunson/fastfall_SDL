#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/engine/time/time.hpp"

#include "fastfall/game/phys/collision/Contact.hpp"
#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

//#include <SFML/Graphics.hpp>

#include <assert.h>
#include <vector>
#include <optional>
#include <memory>

#include "fastfall/game/phys/Identity.hpp"

namespace ff {


class Collidable {
private:

public:

	Collidable();
	Collidable(Vec2f position, Vec2f size, Vec2f gravity = Vec2f{});
	~Collidable();

	Collidable(const Collidable& rhs);
	Collidable(Collidable&& rhs) noexcept;

	Collidable& operator=(const Collidable& rhs);
	Collidable& operator=(Collidable&& rhs) noexcept;

	void init(Vec2f position, Vec2f size, Vec2f gravity = Vec2f{});
	void update(secs deltaTime);

	Rectf getBoundingBox();

	inline Rectf getBox() const noexcept { return curRect; };
	inline Rectf getPrevBox() const noexcept { return prevRect; };

	inline const Vec2f& getPosition() const noexcept { return pos; };
	inline const Vec2f& getPrevPosition() const noexcept { return prevPos; };

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

	void applyContact(const Contact& contact, ContactType type);

	void debug_draw() const;

	// -----------------------------------------

	inline std::vector<std::unique_ptr<SurfaceTracker>>& get_trackers() noexcept { return trackers; };
	inline const std::vector<std::unique_ptr<SurfaceTracker>>& get_trackers() const noexcept { return trackers; };

	SurfaceTracker& create_tracker(Angle ang_min, Angle ang_max, bool inclusive = true);
	SurfaceTracker& create_tracker(Angle ang_min, Angle ang_max, SurfaceTracker::Settings settings, bool inclusive = true);
	bool remove_tracker(SurfaceTracker& tracker);

	// -----------------------------------------

	const PersistantContact* get_contact(Angle angle) const noexcept;
	const PersistantContact* get_contact(Cardinal dir) const noexcept;

	bool has_contact(Angle angle) const noexcept;
	bool has_contact(Cardinal dir) const noexcept;

	inline const std::vector<PersistantContact>& get_contacts() const noexcept { return currContacts; };

	void set_frame(std::vector<PersistantContact>&& frame);

	inline bool is_crush_any() const noexcept { return hori_crush || vert_crush; };
	inline bool is_crush_hori() const noexcept { return hori_crush; };
	inline bool is_crush_vert() const noexcept { return vert_crush; };

	inline CollidableID get_ID() const noexcept { return id; };

	enum class SlipState {
		SlipHorizontal,
		SlipVertical
	};

	struct slip_t {
		SlipState state = SlipState::SlipHorizontal;
		float leeway 	= 0.f;
	};

	void setSlip(slip_t set) { slip = set; };

	bool hasSlip() const noexcept { return slip.leeway != 0.f; };
	bool hasSlipH() const noexcept { return hasSlip() && slip.state == SlipState::SlipHorizontal; };
	bool hasSlipV() const noexcept { return hasSlip() && slip.state == SlipState::SlipVertical; };
	slip_t getSlip() const noexcept { return slip; }


	struct callbacks_t {
		std::function<void()> onPostCollision;
	} callbacks;

private:

	slip_t slip;

	bool hori_crush = false;
	bool vert_crush = false;
	bool wedged = false;

	void process_current_frame();

	CollidableID id;

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

	std::vector<PersistantContact> currContacts;
	std::vector<std::unique_ptr<SurfaceTracker>> trackers;
};

}

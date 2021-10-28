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
	enum class SlipState {
		SlipNone,
		SlipHorizontal,
		SlipVertical
	};

public:



	Collidable(GameContext game_context);
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
	inline void move(Vec2f offset) { setPosition(getPosition() + offset, false); };

	void setPosition(Vec2f position, bool swapPrev = true) noexcept;
	void setSize(Vec2f size) noexcept;
	void teleport(Vec2f position) noexcept;

	inline Vec2f get_gravity() const noexcept { return gravity_acc; };
	inline void set_gravity(Vec2f grav) noexcept { gravity_acc = grav; };

	inline Vec2f get_vel()   const noexcept { return vel; };
	inline void  set_vel(Vec2f velocity) noexcept { vel = velocity; };
	inline void  set_velX(float velocity) noexcept { vel.x = velocity; };
	inline void  set_velY(float velocity) noexcept { vel.y = velocity; };

	inline void add_accel(Vec2f acceleration) { accel_accum += acceleration; };
	inline void add_accelX(float acceleration) { accel_accum.x += acceleration; };
	inline void add_accelY(float acceleration) { accel_accum.y += acceleration; };

	inline void add_decel(Vec2f deceleration) { decel_accum += deceleration; };
	inline void add_decelX(float deceleration) { decel_accum.x += deceleration; };
	inline void add_decelY(float deceleration) { decel_accum.y += deceleration; };

	inline Vec2f get_acc() { return acc; };

	void applyContact(const Contact& contact, ContactType type);

	void debug_draw() const;

	// -----------------------------------------

	inline std::vector<std::unique_ptr<SurfaceTracker>>& get_trackers() noexcept { return trackers; };
	inline const std::vector<std::unique_ptr<SurfaceTracker>>& get_trackers() const noexcept { return trackers; };

	SurfaceTracker& create_tracker(Angle ang_min, Angle ang_max, bool inclusive = true);
	SurfaceTracker& create_tracker(Angle ang_min, Angle ang_max, SurfaceTracker::Settings settings, bool inclusive = true);
	bool remove_tracker(SurfaceTracker& tracker);

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

	void setSlipNone() { slipState = SlipState::SlipNone; slipLeeway = 0.f; };
	void setSlipH(float leeway) { slipState = SlipState::SlipHorizontal; slipLeeway = leeway; };
	void setSlipV(float leeway) { slipState = SlipState::SlipVertical; slipLeeway = leeway; };

	float getSlipH() const noexcept { return slipState == SlipState::SlipHorizontal ? slipLeeway : 0.f; };
	float getSlipV() const noexcept { return slipState == SlipState::SlipVertical ? slipLeeway : 0.f; };

	template<class Fun>
	requires std::is_invocable_v<Fun>
	void set_onPostCollision(Fun&& fun) {
		onPostCollision = fun;
	}

	void clear_onPostCollision() {
		onPostCollision = std::function<void()>{};
	}

private:
	SlipState slipState = SlipState::SlipNone;
	float slipLeeway = 0.f;

	bool hori_crush = false;
	bool vert_crush = false;

	void process_current_frame();

	CollidableID id;
	GameContext context;

	Vec2f pos;
	Rectf curRect;
	Rectf prevRect;

	Vec2f vel;
	Vec2f pVel;

	Vec2f friction;

	Vec2f acc;
	Vec2f gravity_acc;
	Vec2f accel_accum;
	Vec2f decel_accum;

	std::function<void()> onPostCollision;

	std::vector<PersistantContact> currContacts;
	std::vector<std::unique_ptr<SurfaceTracker>> trackers;

};



}
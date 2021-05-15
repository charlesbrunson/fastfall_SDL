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

#include "fastfall/game/phys/Identity.hpp"

namespace ff {

class Collidable {
public:

	static constexpr float REST_THRESHOLD = 1.f * 1.f;

	Collidable();
	~Collidable();

	void init(Vec2f position, Vec2f size);
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

	// -----------------------------------------

	inline std::vector<SurfaceTracker*>& get_trackers() noexcept { return trackers; };
	inline const std::vector<SurfaceTracker*>& get_trackers() const noexcept { return trackers; };

	void add_tracker(SurfaceTracker& tracker);
	void remove_tracker(SurfaceTracker& tracker);

	// will return nullptr if no contact in the given range, or no record for that range

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

private:

	CollidableID id;

	Vec2f gravity_acc;

	Vec2f vel;
	Vec2f accel_accum;
	Vec2f decel_accum;

	Vec2f acc;

	Vec2f pos;
	Rectf curRect;
	Rectf prevRect;

	Vec2f pVel;

	// -----------------------------------------

	bool hori_crush = false;
	bool vert_crush = false;

	void process_current_frame();

	std::vector<PersistantContact> currContacts;
	std::vector<SurfaceTracker*> trackers;

};

}
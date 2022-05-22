#include "fastfall/game/phys/Collidable.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/engine/config.hpp"
#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/render/ShapeCircle.hpp"

#include "fastfall/game/phys/collision/Response.hpp"

#include <assert.h>
#include <functional>

#include "fastfall/game/InstanceInterface.hpp"

namespace ff {

void drawDrawCollidable(const Collidable& c) {
	if (!debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_COLLIDABLE))
		return;
	
	Rectf curRect = c.getBox();
	Rectf prevRect = c.getPrevBox();

	auto& curShape  = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_COLLIDABLE>(Primitive::TRIANGLE_STRIP, 4);
	auto& prevShape = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_COLLIDABLE>(Primitive::LINE_LOOP, 4);
	auto& bounding  = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_COLLIDABLE>(Primitive::LINE_LOOP, 4);

	for (int i = 0; i < curShape.size(); i++) {
		curShape[i].color = Color(0, 255, 0, 100);
	}
	for (int i = 0; i < prevShape.size(); i++) {
		prevShape[i].color = Color(0, 255, 0, 200);
	}
	for (int i = 0; i < bounding.size(); i++) {
		bounding[i].color = Color(255, 255, 0, 200);
	}

	
	prevShape[0].pos = math::rect_topleft(prevRect);
	prevShape[1].pos = math::rect_topright(prevRect);
	prevShape[2].pos = math::rect_botright(prevRect);
	prevShape[3].pos = math::rect_botleft(prevRect);
	//prevShape[4].position = prevShape[0].position;
	

	curShape[0].pos = math::rect_topleft(curRect);
	curShape[1].pos = math::rect_topright(curRect);
	curShape[2].pos = math::rect_botleft(curRect);
	curShape[3].pos = math::rect_botright(curRect);

	Rectf r = math::rect_bound(
		Rectf(curShape[0].pos, curShape[3].pos - curShape[0].pos),
		Rectf(prevShape[0].pos, prevShape[2].pos - prevShape[0].pos)
	);
	bounding[0].pos = math::rect_topleft(r);
	bounding[1].pos = math::rect_topright(r);
	bounding[2].pos = math::rect_botright(r);
	bounding[3].pos = math::rect_botleft(r);
	//bounding[4].position = bounding[0].position;
	
}

void debugDrawContact(const Contact& contact) {

	if (contact.position == Vec2f() ||
		!debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_CONTACT))
		return;

	auto& pos = createDebugDrawable<ShapeCircle, debug_draw::Type::COLLISION_CONTACT>(
		contact.position, 
		1.5f, 
		4, 
		Color::Transparent, 
		Color::Yellow
	);

	auto& line = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_CONTACT>(Primitive::LINES, 4);
	line[0].pos = contact.position;
	line[1].pos = contact.position + (contact.ortho_n * contact.separation);
	line[0].color = Color::Yellow;
	line[1].color = Color::Yellow;

	if (contact.collider.surface.p1 == contact.collider.surface.p2) {

		auto& corner = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_CONTACT>(Primitive::TRIANGLES, 3);
		corner[0].color = Color::White;
		corner[1].color = Color::White;
		corner[2].color = Color::White;

		corner[0].pos = contact.collider.surface.p1;
		corner[1].pos = contact.collider.surface.p1 - (contact.ortho_n - contact.ortho_n.lefthand()) * 2.f;
		corner[2].pos = contact.collider.surface.p1 - (contact.ortho_n + contact.ortho_n.lefthand()) * 2.f;
	}
	else {
		auto& surf = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_CONTACT>(Primitive::TRIANGLES, 18);

		Linef g1 = { contact.collider.ghostp0, contact.collider.surface.p1 };
		Linef g2 = { contact.collider.surface.p2, contact.collider.ghostp3 };

		auto draw_surf = [&](Linef s, size_t start_ndx, Color left, Color right)
		{
			Vec2f n = math::vector(s).lefthand().unit() * 0.5f;

			surf[start_ndx + 0].pos = s.p1;
			surf[start_ndx + 1].pos = s.p2;
			surf[start_ndx + 2].pos = s.p1 - n;
			surf[start_ndx + 3].pos = s.p1 - n;
			surf[start_ndx + 4].pos = s.p2;
			surf[start_ndx + 5].pos = s.p2 - n;

			surf[start_ndx + 0].color = left;
			surf[start_ndx + 1].color = right;
			surf[start_ndx + 2].color = left;
			surf[start_ndx + 3].color = left;
			surf[start_ndx + 4].color = right;
			surf[start_ndx + 5].color = right;
		};

		draw_surf(g1, 0, (contact.collider.g0virtual ? Color::Blue : Color::White), Color::White);
		draw_surf(contact.collider.surface, 6, Color::White, Color::White);
		draw_surf(g2, 12, Color::White, (contact.collider.g3virtual ? Color::Blue : Color::White));
	}
}

Collidable::Collidable(Vec2f position, Vec2f size, Vec2f gravity)
{
	// reserving zero as invalid
	static unsigned collidableIDCounter = CollidableID::NO_ID + 1u;

	id = CollidableID{ collidableIDCounter++ };
	assert(id.value != CollidableID::NO_ID);

	//init(position, size, gravity);
	if (size.x > TILESIZE_F)
		LOG_WARN("{} collidable width > {} not recommended, may break collision", size.x, TILESIZE_F);

	Vec2f topleft = position - Vec2f(size.x / 2.f, size.y);

	curRect = Rectf(topleft, size);
	prevRect = curRect;
	pos = Vec2f(curRect.getPosition()) + Vec2f(curRect.width / 2, curRect.height);
	prevPos = pos;
	gravity_acc = gravity;
}

Collidable::~Collidable() {
	for (auto& track : trackers) {
		track->owner = nullptr;
	}
}

Collidable::Collidable(const Collidable& rhs)
{
	id = rhs.id;
	gravity_acc = rhs.gravity_acc;
	vel = rhs.vel;
	accel_accum = rhs.accel_accum;
	decel_accum = rhs.decel_accum;
	acc = rhs.acc;
	pos = rhs.pos;
	prevPos = rhs.prevPos;
	curRect = rhs.curRect;
	prevRect = rhs.prevRect;
	precollision_vel = rhs.precollision_vel;
	currContacts = rhs.currContacts;

	for (auto& t : rhs.trackers) {
		trackers.push_back(std::make_unique<SurfaceTracker>(*t.get()));
		trackers.back()->owner = this;
	}
}
Collidable::Collidable(Collidable&& rhs) noexcept
{
	id = rhs.id;
	gravity_acc = rhs.gravity_acc;
	vel = rhs.vel;
	accel_accum = rhs.accel_accum;
	decel_accum = rhs.decel_accum;
	acc = rhs.acc;
	pos = rhs.pos;
	prevPos = rhs.prevPos;
	curRect = rhs.curRect;
	prevRect = rhs.prevRect;
	precollision_vel = rhs.precollision_vel;
	std::swap(currContacts, rhs.currContacts);
	std::swap(trackers, rhs.trackers);
}

Collidable& Collidable::operator=(const Collidable& rhs) 
{
	id = rhs.id;
	gravity_acc = rhs.gravity_acc;
	vel = rhs.vel;
	accel_accum = rhs.accel_accum;
	decel_accum = rhs.decel_accum;
	acc = rhs.acc;
	pos = rhs.pos;
	prevPos = rhs.prevPos;
	curRect = rhs.curRect;
	prevRect = rhs.prevRect;
	precollision_vel = rhs.precollision_vel;
	currContacts = rhs.currContacts;

	for (auto& t : rhs.trackers) {
		trackers.push_back(std::make_unique<SurfaceTracker>(*t.get()));
		trackers.back()->owner = this;
	}
	return *this;
}
Collidable& Collidable::operator=(Collidable&& rhs) noexcept
{
	id = rhs.id;
	gravity_acc = rhs.gravity_acc;
	vel = rhs.vel;
	accel_accum = rhs.accel_accum;
	decel_accum = rhs.decel_accum;
	acc = rhs.acc;
	pos = rhs.pos;
	prevPos = rhs.prevPos;
	curRect = rhs.curRect;
	prevRect = rhs.prevRect;
	precollision_vel = rhs.precollision_vel;
	std::swap(currContacts, rhs.currContacts);
	std::swap(trackers, rhs.trackers);
	return *this;
}


void Collidable::update(secs deltaTime) {

	Vec2f prev_pos = pos;
	Vec2f next_pos = pos;

	if (deltaTime > 0.0) {

		vel -= friction;
		acc = accel_accum;

		for (auto& tracker : trackers) {
			CollidableOffsets offsets = tracker->premove_update(deltaTime);

			next_pos += offsets.position;
			vel += offsets.velocity;
			acc += offsets.acceleration;

			if (tracker->has_contact()) {
				tracker->contact_time += deltaTime;
				tracker->air_time = 0.0;
			}
			else {
				tracker->air_time += deltaTime;
			}
		}

		Vec2f surfaceVel;
		for (auto& tracker : trackers) {
			if (tracker->has_contact()) {
				surfaceVel += tracker->currentContact->getSurfaceVel();
			}
		}

		vel += acc * deltaTime;
		vel.x = math::reduce(vel.x, decel_accum.x * (float)deltaTime, surfaceVel.x);
		vel.y = math::reduce(vel.y, decel_accum.y * (float)deltaTime, surfaceVel.y);

		next_pos += vel * deltaTime;

		// perform post move before applying gravity
		for (auto& tracker : trackers) {
			CollidableOffsets offsets = tracker->postmove_update(next_pos, prev_pos);
			next_pos += offsets.position;
			vel += offsets.velocity;
			acc += offsets.acceleration;
		}

		vel += gravity_acc * deltaTime;
		next_pos += gravity_acc * deltaTime * deltaTime;


		precollision_vel = vel;
		setPosition(next_pos);

		accel_accum = Vec2f{};
		decel_accum = Vec2f{};
	}
}

Rectf Collidable::getBoundingBox() {

	Rectf box = getBox();
	Rectf prev = getPrevBox();

	float top = std::min(box.top, prev.top);
	float left = std::min(box.left, prev.left);
	float right = std::max(box.left + box.width, prev.left + prev.width);
	float bottom = std::max(box.top + box.height, prev.top + prev.height);

	return Rectf(left, top, right - left, bottom - top);
};

void Collidable::setPosition(Vec2f position, bool swapPrev) noexcept {
	if (swapPrev) {
		prevRect = curRect;
		prevPos = Vec2f(prevRect.getPosition()) + Vec2f(prevRect.width / 2, prevRect.height);
	}

	assert(!std::isnan(position.x) && !std::isnan(position.y));

	curRect.left = position.x - (curRect.width / 2.f);
	curRect.top = position.y - curRect.height;
	pos = Vec2f(curRect.getPosition()) + Vec2f(curRect.width / 2, curRect.height);
};

void Collidable::setSize(Vec2f size) noexcept {
	if (size.x > TILESIZE_F)
		LOG_WARN("{} collidable width > {} not recommended, may break collision", size.x, TILESIZE_F);

	Vec2f s(abs(size.x), abs(size.y));
	s.x = std::max(s.x, 1.f);
	s.y = std::max(s.y, 1.f);

	Vec2f diff = s - Vec2f(curRect.getSize());

	curRect.left -= diff.x / 2.f;
	curRect.top -= diff.y / 2.f;

	curRect.width = s.x;
	curRect.height = s.y;
	pos = Vec2f(curRect.getPosition()) + Vec2f(curRect.width / 2, curRect.height);
}

void Collidable::teleport(Vec2f position) noexcept {

	curRect.left = position.x - (curRect.width / 2.f);
	curRect.top = position.y - curRect.height;
	prevRect = curRect;
	pos = Vec2f(curRect.getPosition()) + Vec2f(curRect.width / 2, curRect.height);
	prevPos = pos;

	for (auto& tracker : trackers) {
		if (tracker->currentContact.has_value()) {
			tracker->end_touch(tracker->currentContact.value());
			tracker->currentContact = std::nullopt;
			tracker->contact_time = 0.0;
		}
	}
}

void Collidable::applyContact(const Contact& contact, ContactType type) {

	//Vec2f offset = contact.ortho_n * (contact.separation + contact.stickOffset);
	Vec2f offset = contact.ortho_n * (contact.separation);

	move(offset, false);

	if ((type == ContactType::CRUSH_VERTICAL) || (type == ContactType::CRUSH_HORIZONTAL)) {

		set_vel(Vec2f{});

	}
	else if (type == ContactType::WEDGE) {
		if (math::dot(vel - contact.velocity, contact.velocity) < 0.f)
		{
		}
		set_vel(contact.velocity);
	}
	else if (math::dot(get_vel() - contact.velocity, contact.collider_n) <= 0.f) {

		set_vel(phys_resp::get(*this, contact));
		
	}

	if (contact.hasImpactTime) {
		//LOG_INFO("ye");
		for (auto& tracker : trackers) {
			tracker->firstCollisionWith(contact);		
		}
	}
}

// --------------------------------------------------

SurfaceTracker& Collidable::create_tracker(Angle ang_min, Angle ang_max, bool inclusive) {

	auto tracker = std::make_unique<SurfaceTracker>(ang_min, ang_max, inclusive);
	tracker->owner = this;
	trackers.push_back(std::move(tracker));
	return *trackers.back().get();
}

SurfaceTracker& Collidable::create_tracker(Angle ang_min, Angle ang_max, SurfaceTracker::Settings settings, bool inclusive) {
	SurfaceTracker& track = create_tracker(ang_min, ang_max, inclusive);
	track.settings = settings;
	return track;
}

bool Collidable::remove_tracker(SurfaceTracker& tracker) {

	auto tracker_iter = trackers.end();
	if (tracker_iter != trackers.cend()) {
		SurfaceTracker* ptr = tracker_iter->get();

		if (ptr->has_contact() && ptr->callbacks.on_end_touch)
			ptr->callbacks.on_end_touch(ptr->currentContact.value());

		trackers.erase(tracker_iter);
		return true;
	}
	return false;
}

// will return nullptr if no contact in the given range, or no record for that range
const PersistantContact* Collidable::get_contact(Angle angle) const noexcept {

	for (const auto& tracker : trackers) {
		if (tracker->angle_range.within_range(angle)) {

			return tracker->currentContact.has_value() ? &tracker->currentContact.value() : nullptr;
		}
	}

	return nullptr;
}

const PersistantContact* Collidable::get_contact(Cardinal dir) const noexcept {
	return get_contact(direction::to_angle(dir));
}

bool Collidable::has_contact(Angle angle) const noexcept {
	auto* contact = get_contact(angle);
	return contact && contact->hasContact;
}
bool Collidable::has_contact(Cardinal dir) const noexcept {
	return has_contact(direction::to_angle(dir));
}

void Collidable::set_frame(std::vector<PersistantContact>&& frame) {

	currContacts = std::move(frame);

	process_current_frame();

}

void Collidable::debug_draw() const 
{
	if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_COLLIDABLE))
		drawDrawCollidable(*this);

	if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_CONTACT)) {
		for (auto& contact : currContacts) {
			debugDrawContact(contact);
		}
	}
}


void Collidable::process_current_frame() 
{
	col_state.reset();
	for (auto& contact : currContacts) {
		switch(contact.type)
		{
		case ContactType::CRUSH_HORIZONTAL: 
			col_state.set_flag(collision_state_t::flags::Crush_H);
			break;
		case ContactType::CRUSH_VERTICAL: 
			col_state.set_flag(collision_state_t::flags::Crush_V);
			break;
		case ContactType::WEDGE: 
			col_state.set_flag(collision_state_t::flags::Wedge);
			break;
		case ContactType::SINGLE: 
			if (auto optd = direction::from_vector(contact.ortho_n)) 
			{
				switch(optd.value()) 
				{
				case Cardinal::N: col_state.set_flag(collision_state_t::flags::Floor); 	break;
				case Cardinal::S: col_state.set_flag(collision_state_t::flags::Ceiling); break;
				case Cardinal::E: col_state.set_flag(collision_state_t::flags::Wall_L); 	break;
				case Cardinal::W: col_state.set_flag(collision_state_t::flags::Wall_R); 	break;
				}
			}
			break;
		default: 
			break;
		}
	}

	if (trackers.empty())
		return;

	friction = Vec2f{};
	for (auto& tracker : trackers) {
		tracker->process_contacts(currContacts);
		friction += tracker->calc_friction(precollision_vel);
	}

	//Vec2f prev = vel;

	if (callbacks.onPostCollision)
		callbacks.onPostCollision();
}


}

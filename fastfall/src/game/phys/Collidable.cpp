#include "fastfall/game/phys/Collidable.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/engine/config.hpp"
#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/render/ShapeCircle.hpp"

#include "fastfall/game/phys/collision/Response.hpp"

#include <assert.h>
#include <functional>

namespace ff {

void drawDrawCollidable(Collidable& c) {
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
		Rectf(curShape[0].pos, curShape[2].pos - curShape[0].pos),
		Rectf(prevShape[0].pos, prevShape[2].pos - prevShape[0].pos)
	);
	bounding[0].pos = math::rect_topleft(r);
	bounding[1].pos = math::rect_topright(r);
	bounding[2].pos = math::rect_botright(r);
	bounding[3].pos = math::rect_botleft(r);
	//bounding[4].position = bounding[0].position;
	
}

void debugDrawContact(Contact& contact) {

	if (contact.position == Vec2f() ||
		!debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_CONTACT))
		return;

	auto& pos = createDebugDrawable<ShapeCircle, debug_draw::Type::COLLISION_CONTACT>(
		contact.position, 
		1.f, 
		8, 
		Color::Transparent, 
		Color::Yellow
	);


	auto& line = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_CONTACT>(Primitive::LINES, 4);
	line[0].pos = contact.position;
	line[1].pos = contact.position + (contact.ortho_normal * contact.separation);
	line[0].color = Color::Yellow;
	line[1].color = Color::Yellow;

	line[2].pos = contact.collider.surface.p1 - contact.collider_normal;
	line[3].pos = contact.collider.surface.p2 - contact.collider_normal;
	line[2].color = Color::White;
	line[3].color = Color::White;

	auto& surf = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_CONTACT>(Primitive::LINE_STRIP, 4);
	surf[0].color = contact.collider.g0virtual ? Color::Blue : Color::White;
	surf[1].color = Color::White;
	surf[2].color = Color::White;
	surf[3].color = contact.collider.g3virtual ? Color::Blue : Color::White;
	surf[0].pos = contact.collider.ghostp0;
	surf[1].pos = contact.collider.surface.p1;
	surf[2].pos = contact.collider.surface.p2;
	surf[3].pos = contact.collider.ghostp3;

	if (contact.collider.surface.p1 == contact.collider.surface.p2) {

		auto& corner = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_CONTACT>(Primitive::TRIANGLES, 3);
		corner[0].color = Color::White;
		corner[1].color = Color::White;
		corner[2].color = Color::White;

		corner[0].pos = contact.collider.surface.p1;
		corner[1].pos = contact.collider.surface.p1 - (contact.ortho_normal - contact.ortho_normal.lefthand()) * 2.f;
		corner[2].pos = contact.collider.surface.p1 - (contact.ortho_normal + contact.ortho_normal.lefthand()) * 2.f;
	}
}



Collidable::Collidable() {
	// reserving zero as invalid
	static unsigned collidableIDCounter = CollidableID::NO_ID + 1u;

	id = CollidableID{ collidableIDCounter++ };
	assert(id.value != CollidableID::NO_ID);
}

Collidable::~Collidable() {
	for (auto& track : trackers) {
		track->owner = nullptr;
	}
}
void Collidable::init(Vec2f position, Vec2f size) {
	if (size.x > TILESIZE_F)
		LOG_WARN("{} collidable width > {} not recommended, may break collision", size.x, TILESIZE_F);

	Vec2f topleft = position - Vec2f(size.x / 2.f, size.y);

	curRect = Rectf(topleft, size);
	prevRect = curRect;
	pos = Vec2f(curRect.getPosition()) + Vec2f(curRect.width / 2, curRect.height);
}

void Collidable::update(secs deltaTime) {


	acc = accel_accum + gravity_acc;

	for (auto& tracker : trackers) {
		acc += tracker->premove_update(deltaTime);
		if (tracker->has_contact()) {
			tracker->duration += deltaTime;
		}
	}

	vel += acc * deltaTime;
	vel.x = math::reduce(vel.x, decel_accum.x * (float)deltaTime, 0.f);
	vel.y = math::reduce(vel.y, decel_accum.y * (float)deltaTime, 0.f);

	pos += vel * deltaTime;

	for (auto& tracker : trackers) {
		pos = tracker->postmove_update(pos, deltaTime);
	}

	if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_COLLIDABLE))
		drawDrawCollidable(*this);

	setPosition(pos);

	pVel = vel;
	accel_accum = Vec2f{};
	decel_accum = Vec2f{};
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

	for (auto& tracker : trackers) {
		if (tracker->currentContact.has_value()) {
			tracker->end_touch(tracker->currentContact.value());
			tracker->currentContact = std::nullopt;
			tracker->duration_reset();
		}
	}
}

void Collidable::applyContact(const Contact& contact, ContactType type) {

	Vec2f offset = contact.ortho_normal * contact.separation;
	move(offset);

	if ((type == ContactType::CRUSH_VERTICAL) || (type == ContactType::CRUSH_HORIZONTAL) || (type == ContactType::WEDGE_OPPOSITE) ||
		(type == ContactType::WEDGE_SAME) || (type == ContactType::WEDGE_WALL)) {

		set_vel(Vec2f{});

	}
	else if (math::dot(get_vel() - contact.velocity, contact.collider_normal) <= 0.f) {

		set_vel(phys_resp::get(*this, contact));
	}
}

// --------------------------------------------------

void Collidable::add_tracker(SurfaceTracker& tracker) {
	assert(tracker.owner == nullptr);

	auto tracker_iter = std::find(trackers.cbegin(), trackers.cend(), &tracker);
	if (tracker_iter == trackers.cend()) {
		trackers.push_back(&tracker);
	}
	tracker.owner = this;
};

void Collidable::remove_tracker(SurfaceTracker& tracker) {

	if (tracker.has_contact())
		tracker.callback_end_touch(tracker.currentContact.value());

	auto tracker_iter = std::find(trackers.cbegin(), trackers.cend(), &tracker);
	if (tracker_iter != trackers.cend()) {
		trackers.erase(tracker_iter);
	}

	tracker.owner = nullptr;
}

// will return nullptr if no contact in the given range, or no record for that range
const PersistantContact* Collidable::get_contact(Angle angle) const noexcept {

	for (const auto& tracker : trackers) {
		if (tracker->is_angle_in_range(angle)) {
			return tracker->currentContact.has_value() ? &tracker->currentContact.value() : nullptr;
		}
	}

	return nullptr;
}

const PersistantContact* Collidable::get_contact(Cardinal dir) const noexcept {
	return get_contact(cardinalToAngle(dir));
}

bool Collidable::has_contact(Angle angle) const noexcept {
	auto* contact = get_contact(angle);
	return contact && contact->hasContact;
}
bool Collidable::has_contact(Cardinal dir) const noexcept {
	return has_contact(cardinalToAngle(dir));
}

void Collidable::set_frame(std::vector<PersistantContact>&& frame) {

	currContacts = frame;

	process_current_frame();
}



void Collidable::process_current_frame() {

	hori_crush = false;
	vert_crush = false;

	
	if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_CONTACT)) {
		for (auto& contact : currContacts) {
			debugDrawContact(contact);
		}
	}
	

	for (auto& contact : currContacts) {
		if (contact.type == ContactType::CRUSH_HORIZONTAL) {
			hori_crush = true;
		}
		else if (contact.type == ContactType::CRUSH_VERTICAL) {
			vert_crush = true;
		}
	}

	if (trackers.empty())
		return;

	Vec2f friction;
	for (auto& tracker : trackers) {
		tracker->process_contacts(currContacts);
		friction += tracker->get_friction(get_vel(), pVel);
	}
	vel -= friction;
}

}
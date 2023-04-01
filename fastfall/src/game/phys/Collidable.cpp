#include "fastfall/game/phys/Collidable.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/engine/config.hpp"
#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/render/drawable/ShapeCircle.hpp"

#include "fastfall/game/phys/collision/Response.hpp"

#include "fastfall/game/phys/ColliderRegion.hpp"
#include "fastfall/game/ComponentID.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/game/imgui_component.hpp"

#include <assert.h>
#include <functional>

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

void debugDrawContact(const AppliedContact& contact) {

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
	if (size.x > TILESIZE_F)
		LOG_WARN("{} collidable width > {} not recommended, may break collision", size.x, TILESIZE_F);

	Vec2f topleft = position - Vec2f(size.x / 2.f, size.y);

	currRect = Rectf(topleft, size);
	prevRect = currRect;
	currPos = Vec2f(currRect.getPosition()) + Vec2f(currRect.width / 2, currRect.height);
	prevPos = currPos;
	gravity_acc = gravity;
}


Collidable::Collidable(const Collidable& rhs)
{
    col_state = rhs.col_state;
    slip = rhs.slip;

    currPos = rhs.currPos;
    prevPos = rhs.prevPos;

    currRect = rhs.currRect;
    prevRect = rhs.prevRect;

    local_vel = rhs.local_vel;
    local_precollision_vel = rhs.local_precollision_vel;
    parent_vel = rhs.parent_vel;

    friction = rhs.friction;
    acc = rhs.acc;
    gravity_acc = rhs.gravity_acc;
    accel_accum = rhs.accel_accum;
    decel_accum = rhs.decel_accum;

    currContacts = rhs.currContacts;
    _tracker = rhs._tracker;
    callbacks = rhs.callbacks;

    if (_tracker) {
        _tracker->update_collidable_ptr(this);
    }
}

Collidable::Collidable(Collidable&& rhs) noexcept
{
    col_state = rhs.col_state;
    slip = rhs.slip;

    currPos = rhs.currPos;
    prevPos = rhs.prevPos;

    currRect = rhs.currRect;
    prevRect = rhs.prevRect;

    local_vel = rhs.local_vel;
    local_precollision_vel = rhs.local_precollision_vel;
    parent_vel = rhs.parent_vel;

    friction = rhs.friction;
    acc = rhs.acc;
    gravity_acc = rhs.gravity_acc;
    accel_accum = rhs.accel_accum;
    decel_accum = rhs.decel_accum;

    currContacts = std::move(rhs.currContacts);
    _tracker = std::move(rhs._tracker);
    callbacks = rhs.callbacks;

    if (_tracker) {
        _tracker->update_collidable_ptr(this);
    }
}

Collidable& Collidable::operator=(const Collidable& rhs)
{
    if (this == &rhs)
        return *this;

    col_state = rhs.col_state;
    slip = rhs.slip;

    currPos = rhs.currPos;
    prevPos = rhs.prevPos;

    currRect = rhs.currRect;
    prevRect = rhs.prevRect;

    local_vel = rhs.local_vel;
    local_precollision_vel = rhs.local_precollision_vel;
    parent_vel = rhs.parent_vel;

    friction = rhs.friction;
    acc = rhs.acc;
    gravity_acc = rhs.gravity_acc;
    accel_accum = rhs.accel_accum;
    decel_accum = rhs.decel_accum;

    currContacts = rhs.currContacts;
    _tracker = rhs._tracker;
    callbacks = rhs.callbacks;

    if (_tracker) {
        _tracker->update_collidable_ptr(this);
    }
    return *this;
}

Collidable& Collidable::operator=(Collidable&& rhs) noexcept
{
    if (this == &rhs)
        return *this;

    col_state = rhs.col_state;
    slip = rhs.slip;

    currPos = rhs.currPos;
    prevPos = rhs.prevPos;

    currRect = rhs.currRect;
    prevRect = rhs.prevRect;

    local_vel = rhs.local_vel;
    local_precollision_vel = rhs.local_precollision_vel;
    parent_vel = rhs.parent_vel;

    friction = rhs.friction;
    acc = rhs.acc;
    gravity_acc = rhs.gravity_acc;
    accel_accum = rhs.accel_accum;
    decel_accum = rhs.decel_accum;

    currContacts = std::move(rhs.currContacts);
    _tracker = std::move(rhs._tracker);
    callbacks = rhs.callbacks;

    if (_tracker) {
        _tracker->update_collidable_ptr(this);
    }
    return *this;
}


void Collidable::update(poly_id_map<ColliderRegion>* colliders, secs deltaTime) {

	Vec2f prev_pos = currPos;
	Vec2f next_pos = currPos;

	if (deltaTime > 0.0)
    {
		local_vel -= friction;
		acc = accel_accum;

		if (_tracker) {
			auto offsets = _tracker->premove_update(colliders, deltaTime);
			next_pos    += offsets.pos_offset;
            local_vel   += offsets.vel_offset;
			acc         += offsets.acc_offset;

            if (_tracker->has_contact()) {
                parent_vel = offsets.parent_vel;
                last_parent_vel = offsets.parent_vel;
            }
		}

        Vec2f zero_vel = _tracker && _tracker->has_contact() ? Vec2f{} : last_parent_vel;

        local_vel += acc * (float)deltaTime;
        local_vel.x = math::reduce(local_vel.x, decel_accum.x * (float)deltaTime, zero_vel.x);
        local_vel.y = math::reduce(local_vel.y, decel_accum.y * (float)deltaTime, zero_vel.y);

        next_pos += get_global_vel() * (float)deltaTime;

		// perform post move before applying gravity
        if (_tracker) {
			auto offsets = _tracker->postmove_update(colliders, next_pos, prev_pos);
			next_pos += offsets.pos_offset;
		}

        local_vel += gravity_acc * deltaTime;
		next_pos += gravity_acc * deltaTime * deltaTime;

		local_precollision_vel = local_vel;
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
		prevRect = currRect;
		prevPos = Vec2f(prevRect.getPosition()) + Vec2f(prevRect.width / 2, prevRect.height);
	}

	assert(!std::isnan(position.x) && !std::isnan(position.y));

    currRect.left = position.x - (currRect.width / 2.f);
    currRect.top = position.y - currRect.height;
	currPos = Vec2f(currRect.getPosition()) + Vec2f(currRect.width / 2, currRect.height);
};

void Collidable::setSize(Vec2f size) noexcept {
	if (size.x > TILESIZE_F)
		LOG_WARN("{} collidable width > {} not recommended, may break collision", size.x, TILESIZE_F);

	Vec2f s(abs(size.x), abs(size.y));
	s.x = std::max(s.x, 1.f);
	s.y = std::max(s.y, 1.f);

	Vec2f diff = s - Vec2f(currRect.getSize());

    currRect.left -= diff.x / 2.f;
    currRect.top -= diff.y / 2.f;

    currRect.width = s.x;
    currRect.height = s.y;
    currPos = Vec2f(currRect.getPosition()) + Vec2f(currRect.width / 2, currRect.height);
}

void Collidable::teleport(Vec2f position) noexcept {

    currRect.left = position.x - (currRect.width / 2.f);
    currRect.top = position.y - currRect.height;
	prevRect = currRect;
    currPos = Vec2f(currRect.getPosition()) + Vec2f(currRect.width / 2, currRect.height);
	prevPos = currPos;

	if (_tracker) {
        _tracker->force_end_contact();
	}
}

void Collidable::applyContact(const AppliedContact& contact, ContactType type)
{
	Vec2f offset = contact.ortho_n * (contact.separation);

	move(offset, false);

	if ((type == ContactType::CRUSH_VERTICAL) || (type == ContactType::CRUSH_HORIZONTAL) || (type == ContactType::WEDGE))
    {
		set_local_vel(Vec2f{});
	}
	else if (math::dot(get_global_vel() - contact.velocity, contact.collider_n) <= 0.f)
    {
        Vec2f resp = phys_resp::get(*this, contact);
        set_local_vel(resp - (get_parent_vel() + get_surface_vel()));
	}

	if (contact.hasImpactTime && _tracker) {
        _tracker->firstCollisionWith(contact);
	}
}

// will return nullptr if no contact in the given range, or no record for that range
const AppliedContact* Collidable::get_contact(Angle angle) const noexcept {

	if (_tracker && _tracker->angle_range.within_range(angle)) {
        return _tracker->get_contact().has_value() ? &_tracker->get_contact().value() : nullptr;
	}

	return nullptr;
}

const AppliedContact* Collidable::get_contact(Cardinal dir) const noexcept {
	return get_contact(direction::to_angle(dir));
}

bool Collidable::has_contact(Angle angle) const noexcept {
	auto* contact = get_contact(angle);
	return contact && contact->hasContact;
}
bool Collidable::has_contact(Cardinal dir) const noexcept {
	return has_contact(direction::to_angle(dir));
}

void Collidable::set_frame(
        poly_id_map<ColliderRegion>* colliders,
        std::vector<AppliedContact>&& frame)
{
	currContacts = std::move(frame);

    // process contacts
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

    friction = Vec2f{};
    if (_tracker) {
        _tracker->process_contacts(colliders, currContacts);
        friction += _tracker->calc_friction(local_precollision_vel);
    }
}

void Collidable::create_tracker(Angle ang_min, Angle ang_max, bool inclusive) {
    _tracker.emplace(this, ang_min, ang_max, inclusive);
}

bool Collidable::erase_tracker() {
    bool exists = _tracker.has_value();
    _tracker.reset();
    reset_parent_vel();
    return exists;
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

Vec2f Collidable::get_gravity() const noexcept { return gravity_acc; };
void  Collidable::set_gravity(Vec2f grav) noexcept { gravity_acc = grav; };

Vec2f Collidable::get_local_vel()   const noexcept { return local_vel; };
Vec2f Collidable::get_parent_vel()  const noexcept { return parent_vel; };
Vec2f Collidable::get_surface_vel() const noexcept { return surface_vel; };
Vec2f Collidable::get_global_vel()  const noexcept { return local_vel + parent_vel + surface_vel; };

void Collidable::set_local_vel(Vec2f velocity) noexcept {
    local_vel = velocity;
};
void Collidable::set_local_vel(std::optional<float> X, std::optional<float> Y) noexcept {
    local_vel.x = X.value_or(local_vel.x);
    local_vel.y = Y.value_or(local_vel.y);
}

void Collidable::apply_parent_vel(Vec2f pvel) noexcept {
    Vec2f pvel_diff = pvel - parent_vel;
    parent_vel = pvel;
    last_parent_vel = pvel;
    local_vel -= pvel_diff;
}

void Collidable::reset_parent_vel() noexcept {
    local_vel += parent_vel;
    parent_vel = Vec2f{};
}

void Collidable::apply_surface_vel(Vec2f svel) noexcept {
    Vec2f svel_diff = svel - surface_vel;
    surface_vel = svel;
    local_vel -= svel_diff;
}

void Collidable::reset_surface_vel() noexcept {
    local_vel += surface_vel;
    surface_vel = Vec2f{};
}

void Collidable::set_parent_vel(Vec2f pvel) noexcept { parent_vel = pvel; }

Vec2f Collidable::get_last_parent_vel() const noexcept { return last_parent_vel; }
void Collidable::set_last_parent_vel(Vec2f pvel) noexcept { last_parent_vel = pvel; }

void Collidable::add_accel(Vec2f acceleration) { accel_accum += acceleration; };
void Collidable::add_decel(Vec2f deceleration) { decel_accum += deceleration; };

Vec2f Collidable::get_friction()   const noexcept { return friction; };
Vec2f Collidable::get_acc() const noexcept { return acc; };

void imgui_component(World& w, ID<Collidable> id) {
    auto& col = w.at(id);

    auto text_vec2 = [](std::string_view label, const auto& v) {
        ImGui::Text("%s: ", label.data());     ImGui::NextColumn();
        ImGui::Text("%3.2f, %3.2f", v.x, v.y); ImGui::NextColumn();
    };
    auto text_vec1 = [](std::string_view label, const auto& v) {
        ImGui::Text("%s: ", label.data());     ImGui::NextColumn();
        ImGui::Text("%3.2f", v); ImGui::NextColumn();
    };

    ImGui::Columns(2);
    text_vec2("Curr Pos",    col.getPosition());
    text_vec2("Prev Pos",    col.getPosition());
    text_vec2("Curr Center", col.getBox().center());
    text_vec2("Prev Center", col.getPrevBox().center());
    text_vec2("Curr Size",   col.getBox().getSize());
    text_vec2("Prev Size",   col.getPrevBox().getSize());

    text_vec2("Local  Vel",      col.get_local_vel());
    text_vec2("Parent Vel",      col.get_parent_vel());
    text_vec2("Last Parent Vel", col.get_last_parent_vel());
    text_vec2("Surface Vel",     col.get_surface_vel());
    text_vec2("Global Vel",      col.get_global_vel());

    text_vec2("Accel",    col.get_acc());
    text_vec2("Friction", col.get_friction());
    text_vec2("Gravity",  col.get_gravity());

    text_vec1("Local  Speed", col.get_local_vel().magnitude());
    text_vec1("Parent Speed", col.get_parent_vel().magnitude());
    text_vec1("Global Speed", col.get_global_vel().magnitude());

    ImGui::Text("Attach ID: "); ImGui::NextColumn();
    imgui_component_ref(w, col.get_attach_id()); ImGui::NextColumn();

    text_vec2("Attach Offset",       col.get_attach_origin());
    ImGui::Columns();

    if (ImGui::TreeNode((void*)(&col.tracker()), "Tracker")) {

        if (!col.tracker()) {
            ImGui::Text("No trackers!");
        }
        else {
            auto& tracker = *col.tracker();

            static char labelbuf[32];
            sprintf(labelbuf, "Friction (%d)", tracker.settings.has_friction);

            if (ImGui::SmallButton(labelbuf)) {
                tracker.settings.has_friction = !tracker.settings.has_friction;
            } ImGui::SameLine();
            sprintf(labelbuf, "Platform Stick (%d)", tracker.settings.move_with_platforms);
            if (ImGui::SmallButton(labelbuf)) {
                tracker.settings.move_with_platforms = !tracker.settings.move_with_platforms;
            } ImGui::SameLine();
            sprintf(labelbuf, "Slope Stick (%d)", tracker.settings.slope_sticking);
            if (ImGui::SmallButton(labelbuf)) {
                tracker.settings.slope_sticking = !tracker.settings.slope_sticking;
            }ImGui::SameLine();
            sprintf(labelbuf, "Wall stop (%d)", tracker.settings.slope_wall_stop);
            if (ImGui::SmallButton(labelbuf)) {
                tracker.settings.slope_wall_stop = !tracker.settings.slope_wall_stop;
            }

            static char trackerbuf[32];
            sprintf(trackerbuf, "%p", &col);
            ImGui::Columns(2, trackerbuf);
            //ImGui::SetColumnWidth(0, 120.f);
            ImGui::Separator();

            ImGui::Text("Angle Range"); ImGui::NextColumn();
            ImGui::Text("(%3.2f, %3.2f)", tracker.angle_range.min.degrees(), tracker.angle_range.max.degrees()); ImGui::NextColumn();

            ImGui::Text("Has Contact"); ImGui::NextColumn();
            ImGui::Text("%s", tracker.has_contact() ? "true" : "false"); ImGui::NextColumn();

            ImGui::Text("Contact Duration"); ImGui::NextColumn();
            ImGui::Text("%3.2f", tracker.contact_time); ImGui::NextColumn();

            ImGui::Text("Air Duration"); ImGui::NextColumn();
            ImGui::Text("%3.2f", tracker.air_time); ImGui::NextColumn();

            ImGui::Text("Traversal Speed"); ImGui::NextColumn();
            auto tspeed = tracker.traverse_get_speed();
            ImGui::Text("%3.2f", tspeed ? *tspeed : 0.f); ImGui::NextColumn();

            ImGui::Text("Max Speed"); ImGui::NextColumn();
            ImGui::Text("%3.2f", tracker.settings.max_speed); ImGui::NextColumn();

            if (tracker.get_contact().has_value()) {
                const auto& c = tracker.get_contact().value();

                ImGui::Text("Collider"); ImGui::NextColumn();
                if (tracker.get_contact()->id) {
                    imgui_component_ref(w, tracker.get_contact()->id->collider);
                }
                else {
                    ImGui::Text("N/A");
                }
                ImGui::NextColumn();

                ImGui::Text("Surface Normal"); ImGui::NextColumn();
                ImGui::Text("(%3.2f, %3.2f)", c.collider_n.x, c.collider_n.y); ImGui::NextColumn();

                ImGui::Text("Ortho Normal"); ImGui::NextColumn();
                ImGui::Text("(%1.f, %1.f)", c.ortho_n.x, c.ortho_n.y); ImGui::NextColumn();

                ImGui::Text("Separation"); ImGui::NextColumn();
                ImGui::Text("%s%3.3f", (c.separation == 0 ? " " : (c.separation > 0 ? "+" : "-")), c.separation); ImGui::NextColumn();

                ImGui::Text("Surface Velocity"); ImGui::NextColumn();
                ImGui::Text("(%3.2f, %3.2f)", c.velocity.x, c.velocity.y); ImGui::NextColumn();

                ImGui::Text("Has Contact"); ImGui::NextColumn();
                ImGui::Text("%s", c.hasContact ? "true" : "false"); ImGui::NextColumn();

                ImGui::Text("Impact Time"); ImGui::NextColumn();
                if (c.hasImpactTime) {
                    ImGui::Text("%.3f", c.impactTime);
                }
                else {
                    ImGui::Text("N/A");
                } ImGui::NextColumn();

                ImGui::Text("Duration"); ImGui::NextColumn();
                ImGui::Text("%3.2f", c.touchDuration); ImGui::NextColumn();

                ImGui::Text("Contact Type"); ImGui::NextColumn();
                ImGui::Text("%s", contactTypeToString(c.type).data()); ImGui::NextColumn();

            }
            ImGui::Columns(1);
            ImGui::Separator();
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNode((void*)(&col.get_contacts()), "Collisions")) {
        bool has_collision = false;
        if (!col.get_contacts().empty()) {

            for (auto& c : col.get_contacts()) {

                if (!has_collision) {

                    static char collisionbuf[32];
                    sprintf(collisionbuf, "%p", &col);
                    ImGui::Columns(2, collisionbuf);
                    //ImGui::SetColumnWidth(0, 120.f);
                    ImGui::Separator();

                    has_collision = true;
                }

                ImGui::Text("Collider: "); ImGui::NextColumn();
                if (c.id && c.id->collider) {
                    imgui_component_ref(w, ComponentID{ c.id->collider });
                }
                else {
                    ImGui::Text("Combined");
                }
                ImGui::NextColumn();

                ImGui::Text("Surface Normal"); ImGui::NextColumn();
                ImGui::Text("(%3.2f, %3.2f)", c.collider_n.x, c.collider_n.y); ImGui::NextColumn();

                ImGui::Text("Ortho Normal"); ImGui::NextColumn();
                ImGui::Text("(%1.f, %1.f)", c.ortho_n.x, c.ortho_n.y); ImGui::NextColumn();

                ImGui::Text("Separation"); ImGui::NextColumn();
                ImGui::Text("%s%3.3f", (c.separation == 0 ? " " : (c.separation > 0 ? "+" : "-")), c.separation); ImGui::NextColumn();

                ImGui::Text("Velocity"); ImGui::NextColumn();
                ImGui::Text("(%3.2f, %3.2f)", c.velocity.x, c.velocity.y); ImGui::NextColumn();

                ImGui::Text("Surface Velocity"); ImGui::NextColumn();
                ImGui::Text("(%3.2f, %3.2f)", c.surface_vel().x, c.surface_vel().y); ImGui::NextColumn();

                ImGui::Text("Has Contact"); ImGui::NextColumn();
                ImGui::Text("%s", c.hasContact ? "true" : "false"); ImGui::NextColumn();

                ImGui::Text("Impact Time"); ImGui::NextColumn();
                if (c.hasImpactTime) {
                    ImGui::Text("%.3f", c.impactTime);
                }
                else {
                    ImGui::Text("N/A");
                } ImGui::NextColumn();

                ImGui::Text("Duration"); ImGui::NextColumn();
                ImGui::Text("%3.2f", c.touchDuration); ImGui::NextColumn();


                ImGui::Text("Contact Type"); ImGui::NextColumn();
                ImGui::Text("%s", contactTypeToString(c.type).data()); ImGui::NextColumn();

                ImGui::Separator();
            }
        }
        else {
            ImGui::Text("None"); ImGui::NextColumn(); ImGui::NextColumn();
            ImGui::Separator();
        }

        if (!has_collision) {
            ImGui::Text("No collisions!");
        }

        ImGui::Columns();
        ImGui::TreePop();
    }
}

}

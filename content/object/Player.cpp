
#include "Player.hpp"

#include "fastfall/game/CollisionManager.hpp"
#include "fastfall/game/TriggerManager.hpp"
#include "fastfall/game/GameCamera.hpp"

#include <functional>

using namespace ff;

namespace constants {

	AnimIDRef idle  ("player", "idle");
	AnimIDRef land  ("player", "land");
	AnimIDRef run   ("player", "running");
	AnimIDRef jump  ("player", "jump");
	AnimIDRef fall  ("player", "fall");

	AnimIDRef brakeb("player", "brake_back");
	AnimIDRef brakef("player", "brake_front");

	const Friction braking{ .stationary = 1.2f, .kinetic = 0.8f };
	const Friction moving { .stationary = 0.f,  .kinetic = 0.f };

	const float max_speed  =  500.f;
	const float norm_speed =  200.f;
	const float jumpVelY   = -200.f;

	const Vec2f grav_normal{ 0.f, 500.f };
	const Vec2f grav_light { 0.f, 350.f };
}

using namespace constants;

Player::Player(GameContext instance, const ObjectRef& ref, const ObjectType& type) 
	: GameObject{ instance, ref, type }
{
	// collision box
	box = context.collision()->create_collidable(Vec2f(ref.position), Vec2f(8.f, 28.f), grav_normal);

	// surface tracker
	ground = &box->create_tracker(Angle::Degree(-135), Angle::Degree(-45), {
			.move_with_platforms = true,
			.slope_sticking = true,
			.slope_wall_stop = true,
			.has_friction = true,
			.stick_angle_max = Angle::Degree(90),
		});

	// camera target
	context.camera()->addTarget({
			.movingTarget = &box->getPosition(),
			.type = GameCamera::TargetType::MOVING,
			.offset = Vec2f(0, -16),
			.priority = GameCamera::TargetPriority::MEDIUM,
		});

	// triggers
	hitbox = context.triggers()->create_trigger(box->getBox(), { "hitbox" }, {}, this);
	hurtbox = context.triggers()->create_trigger(box->getBox(), { "hurtbox" }, { "hitbox" }, this);
	hurtbox->set_trigger_callback([](const TriggerPull& pull) {
			if (auto owner = pull.trigger.get().get_owner()) {
				if (GameObject* obj = *owner; obj->getType().group_tags.contains("player")) {
					switch (pull.state) {
					case Trigger::State::Entry: LOG_INFO("ENTER"); obj->command<ObjCmd::NoOp>(); break;
					case Trigger::State::Loop:  LOG_INFO("LOOP");  break;
					case Trigger::State::Exit:  LOG_INFO("EXIT");  break;
					}
				}
			}
		});

	// sprite
	sprite.set_anim(idle);
	sprite.set_pos(box->getPosition());

	drawPriority = 0;
};

Player::~Player() {
	if (context.valid()) {
		context.collision()->erase_collidable(box);
		context.camera()->removeTarget(GameCamera::TargetPriority::MEDIUM);
		context.triggers()->erase_trigger(hitbox);
		context.triggers()->erase_trigger(hurtbox);
	}
}

std::unique_ptr<GameObject> Player::clone() const {

	std::unique_ptr<Player> object = std::make_unique<Player>(context, getObjectRef(), getType());

	//TODO copy current state data

	return object;
}

void Player::update(secs deltaTime) {

	int wishx = (int)Input::isHeld(InputType::RIGHT) - (int)Input::isHeld(InputType::LEFT);

	sprite.set_playback(1.f);

	box->set_gravity(grav_normal);

	// on ground
	if (ground->has_contact()) {
		box->setSlipNone();
		const PersistantContact& contact = ground->get_contact().value();

		ground->settings.slope_sticking = true;

		float speed = ground->traverse_get_speed();

		if (ground->get_contact_time() == 0.f) {

			ground->settings.max_speed = std::max(norm_speed, std::min(std::abs(speed), max_speed));
			sprite.set_anim(land);
		}
		else {
			ground->settings.max_speed = std::max(norm_speed, std::min(std::abs(speed), ground->settings.max_speed));


			if (abs(speed) > norm_speed) {
				ground->traverse_add_decel(300.f);
			}
		}

		int movex = (speed == 0.f ? 0 : (speed < 0.f ? -1 : 1));

		if (abs(speed) <= 100.f && wishx == 0
			|| abs(speed) <= 5.f && wishx != 0) {
			sprite.set_anim_if_not(idle);
		}
		else {


			if ((wishx < 0) == (movex < 0)) {
				if (wishx != 0) {
					sprite.set_hflip(movex < 0);
				}

				sprite.set_anim_if_not(run);
				sprite.set_playback(
					std::clamp(abs(speed) / 150.f, 0.5f, 2.f)
				);
			}
		}

		// jumping
		if (Input::isPressed(InputType::JUMP, 0.1f)) {
			Input::confirmPress(InputType::JUMP);

			sprite.set_anim(jump);
			ground->settings.slope_sticking = false;

			Vec2f jumpVel = Vec2f{ box->get_vel().x, jumpVelY} ;
			Angle jump_ang = math::angle(jumpVel) - math::angle(ground->get_contact()->collider_normal);

			// from perpendicular to the ground
			static const Angle min_jump_ang = Angle::Degree(60);

			if (jump_ang < -min_jump_ang) {
				jumpVel = math::rotate(jumpVel, -jump_ang - min_jump_ang);
			}
			else if (jump_ang > min_jump_ang) {
				jumpVel = math::rotate(jumpVel, -jump_ang + min_jump_ang);
			}
			box->set_vel(jumpVel + Vec2f{ 0.f, ground->get_contact()->velocity.y });


			if (wishx != 0) {
				sprite.set_hflip(wishx < 0);
			}

		}
		else {

			bool turning = wishx != 0 && ((movex < 0) != (wishx < 0));
			bool nowishx = wishx == 0;
			bool shouldbrake = nowishx || turning;

			if (movex != 0 && abs(speed) > 100.f && shouldbrake) {
				AnimID brakeStyle = sprite.get_hflip() == (movex < 0) ? brakef.id() : brakeb.id();

				if (abs(speed) > 300.f) {
					sprite.set_anim(brakeStyle);
				}
				else if (!(sprite.is_playing(brakeStyle) && sprite.get_frame() == 0)) {
					sprite.set_anim(brakeStyle);
					sprite.set_frame(1);
				}
			}

			if (wishx != 0) {
				ground->traverse_add_accel(wishx * 1200.f);
				ground->settings.surface_friction = turning || nowishx ? braking : moving;
			}
			else if (ground->settings.has_friction) {
				ground->traverse_add_decel(450.f);
				ground->settings.surface_friction = braking;
			}
		}
	}
	// in air
	else {
		if (box->get_vel().y > -100.f) {
			box->setSlipV(6.f);
		}
		else {
			box->setSlipNone();
		}

		if (abs(box->get_vel().y) < 50.f) {
			box->set_gravity(grav_light);
		}

		// flight control
		int wishy = (int)Input::isHeld(InputType::DOWN) - (int)Input::isHeld(InputType::UP);
		box->add_accelY(600.f * wishy);

		if (ground->get_air_time() > 0.05
			&& !sprite.is_playing_any({ jump, fall }))
		{
			sprite.set_anim(fall);
			sprite.set_frame(2);
		}
		else if (sprite.is_complete(jump)
			&& box->get_vel().y > -100.f) 
		{
			sprite.set_anim(fall);
		}

		ground->settings.slope_sticking = false;
		// air control
		if (wishx != 0 && (abs(box->get_vel().x) < 150.f ||
			box->get_vel().x < 0.f != wishx < 0.f)) {
			box->add_accelX(500.f * wishx);
		}
	}

	if (Input::isPressed(InputType::ATTACK)) {
		box->setSize(Vec2f(box->getBox().getSize()) + Vec2f(1.f, 1.f));
	}
	if (Input::isPressed(InputType::DASH)) {
		box->setSize(Vec2f(box->getBox().getSize()) + Vec2f(-1.f, -1.f));
	}

	// blink
	if (Input::getMouseInView() && Input::isPressed(InputType::MOUSE1)) {
		box->teleport(Input::getMouseWorldPosition());
		box->set_vel(Vec2f{});
	}

	box->update(deltaTime);
	sprite.update(deltaTime);
	hitbox->update(box->getBox());
	hurtbox->update();
}

void Player::predraw(secs deltaTime) {
	sprite.set_pos(box->getPosition());
	sprite.predraw(deltaTime);
}

void Player::ImGui_Inspect() {
	ImGui::Text("Hello World!");
	ImGui::Text("Position(%3.2f, %3.2f)", box->getPosition().x, box->getPosition().y);
	ImGui::Text("Velocity(%3.2f, %3.2f)", box->get_vel().x, box->get_vel().y);
}

void Player::draw(RenderTarget& target, RenderState states) const {
	target.draw(sprite, states);
}

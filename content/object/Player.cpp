
#include "Player.hpp"

GameObjectLibrary::Entry<Player> plr_type{ 
	{
		.tile_size = {1, 2},
		.properties = {
			ObjectTypeProperty("anotherprop", ObjectPropertyType::String),
			ObjectTypeProperty("faceleft", false)
		}
	} 
};

namespace constants {

	AnimIDRef a_idle("player", "idle");
	AnimIDRef a_land("player", "land");
	AnimIDRef a_run("player", "running");
	AnimIDRef a_jump("player", "jump");
	AnimIDRef a_fall("player", "fall");

	AnimIDRef a_jump_f("player", "jump_f");
	AnimIDRef a_fall_f("player", "fall_f");
}

using namespace constants;

Player::Player(GameContext instance, const ObjectRef& ref) :
	GameObject{ instance, ref },
	ground{ instance, Angle::Degree(-135), Angle::Degree(-45) }
{
	Collidable myBox;
	myBox.init(Vec2f(ref.position), Vec2f(8.f, 28.f));
	box = context.collision()->createCollidable(std::move(myBox));
	box->set_gravity(Vec2f{ 0.f, 450.f });

	ground.has_friction = true;
	ground.move_with_platforms = true;
	ground.slope_sticking = true;
	ground.slope_wall_stop = true;
	ground.stick_angle_max = Angle::Degree(90);
	box->add_tracker(ground);

	braking = Friction{ .stationary = 1.2f, .kinetic = 0.8f };
	moving = Friction{ .stationary = 0.f,  .kinetic = 0.f };

	GameCamera::Target target;
	target.priority = GameCamera::TargetPriority::MEDIUM;
	target.type = GameCamera::TargetType::MOVING;
	target.movingTarget = &box->getPosition();
	target.offset = Vec2f(0, -16);
	context.camera()->addTarget(target);

	drawPriority = 0;

	sprite.set_anim(a_idle.id());

	sprite.set_pos(box->getPosition());
};

Player::~Player() {
	if (context.valid()) {
		context.collision()->removeCollidable(box);
		context.camera()->removeTarget(GameCamera::TargetPriority::MEDIUM);
	}
}

std::unique_ptr<GameObject> Player::clone() const {

	std::unique_ptr<Player> object = std::make_unique<Player>(context, *getObjectRef());

	//TODO copy current state data

	return object;
}

void Player::update(secs deltaTime) {

	int wishx = (int)Input::isHeld(InputType::RIGHT) - (int)Input::isHeld(InputType::LEFT);

	sprite.set_playback(1.f);

	//raycast(context.collision(), box->getPosition(), Cardinal::SOUTH);

	// on ground
	if (ground.has_contact()) {
		const PersistantContact& contact = ground.get_contact().value();

		Vec2f groundUnit = contact.collider_normal.righthand();
		ground.slope_sticking = true;

		float speed = ground.traverse_get_speed();
		const static float max_speed = 600.f;
		const static float norm_speed = 250.f;


		if (ground.get_duration() == 0.0) {
			ground.traverse_set_max_speed(
				std::max(norm_speed, std::min(std::abs(speed), max_speed))
			);
			sprite.set_anim(a_land.id());
		}
		else {
			ground.traverse_set_max_speed(
				std::max(norm_speed, std::min(std::abs(speed), ground.traverse_get_max_speed()))
			);

			if (abs(speed) > norm_speed) {
				ground.traverse_add_decel(300.f);
			}
		}

		int movex = (speed == 0.f ? 0 : (speed < 0.f ? -1 : 1));

		airtime = 0.0;

		// sinful
		//float target_rot = -math::angle(ground.get_contact()->collider.surface).radians() / 3.f;
		//target_rot = (target_rot * 0.1f) + (sprite.get_sprite().getRotation() * 0.9f);
		//sprite.get_sprite().setRotation(target_rot);


		if (abs(box->get_vel().x) <= 100.f && wishx == 0) {
			sprite.set_anim_if_not(a_idle.id());
		}
		else {
			sprite.set_anim_if_not(a_run.id());

			sprite.set_playback(
				std::max(
					0.5f,
					std::abs(box->get_vel().magnitude()) / 200.f
				));
		}

		// jumping
		if (Input::isPressed(InputType::JUMP, 0.1f)) {
			Input::confirmPress(InputType::JUMP);

			if (wishx == movex && movex != 0 && abs(speed) >= 200.f) {
				sprite.set_anim(a_jump_f.id());
			}
			else {
				sprite.set_anim(a_jump.id());
			}

			ground.slope_sticking = false;


			Vec2f jumpVel = Vec2f{ box->get_vel().x, -200.f };

			Angle jump_ang = math::angle(jumpVel) - math::angle(ground.get_contact()->collider_normal);

			// from perpendicular to the ground
			static const Angle min_jump_ang = Angle::Degree(60);

			if (jump_ang < -min_jump_ang) {
				jumpVel = math::rotate(jumpVel, -jump_ang - min_jump_ang);
			}
			else if (jump_ang > min_jump_ang) {
				jumpVel = math::rotate(jumpVel, -jump_ang + min_jump_ang);
			}
			box->set_vel(jumpVel);

		}
		else {
			//ground.traverse_set_max_speed(0.f);

			if (wishx != 0) {
				ground.traverse_add_accel(wishx * 1200.f);

				bool isBraking = (movex != 0) && (wishx < 0) != (movex < 0);
				ground.surface_friction = isBraking ? braking : moving;

			}
			else if (ground.has_friction) {
				ground.traverse_add_decel(600.f);
				ground.surface_friction = braking;
			}
		}
	}
	// in air
	else {
		// sinful
		//float target_rot = (sprite.get_sprite().getRotation() * 0.9f);
		//sprite.get_sprite().setRotation(target_rot);

		airtime += deltaTime;

		// flight control
		int wishy = (int)Input::isHeld(InputType::DOWN) - (int)Input::isHeld(InputType::UP);
		box->add_accelY(600.f * wishy);

		if (airtime > 0.05
			&& !sprite.is_playing_any({ a_jump.id(), a_jump_f.id() })
			&& !sprite.is_playing_any({ a_fall.id(), a_fall_f.id() })
			)
		{
			sprite.set_anim(a_fall.id());

			sprite.set_frame(2);
		}
		else if ( (sprite.is_complete(a_jump.id()) || sprite.is_complete(a_jump_f.id()))
			&& box->get_vel().y > -100.f) {

			if (sprite.is_playing(a_jump.id())) {
				sprite.set_anim(a_fall.id());
			}
			else {
				sprite.set_anim(a_fall_f.id());
			}
		}

		ground.slope_sticking = false;
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
	if (box->get_vel().x != 0) {
		sprite.set_hflip(box->get_vel().x < 0);
	}
}

void Player::predraw(secs deltaTime) {
	sprite.set_pos(box->getPosition());
	sprite.predraw(deltaTime);
}

void Player::ImGui_Inspect() {
	ImGui::Text("Hello World!");
	ImGui::Text("Position(%3.2f, %3.2f)", box->getPosition().x, box->getPosition().y);
	ImGui::Text("Velocity(%3.2f, %3.2f)", box->get_vel().x, box->get_vel().y);

	//ImGui::Text("Braking: %s", (isBraking ? "true" : "false"));

}

void Player::draw(RenderTarget& target, RenderState states) const {
	target.draw(sprite, states);
}

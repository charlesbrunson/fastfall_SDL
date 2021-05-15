#pragma once

#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/game/phys/Collidable.hpp"

#include "fastfall/engine/input.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/game/phys/Raycast.hpp"

#include "fastfall/game/CollisionManager.hpp"
#include "fastfall/game/GameCamera.hpp"

#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/resource/Resources.hpp"

using namespace ff;

class Player : public GameObject {
public:
	Player(GameContext instance, const ObjectRef& ref) :
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
		//ground.traverse_set_max_speed(150.f);
		box->add_tracker(ground);

		braking = Friction{ .stationary = 1.2f, .kinetic = 0.8f };
		moving  = Friction{ .stationary = 0.f,  .kinetic = 0.f  };

		GameCamera::Target target;
		target.priority = GameCamera::TargetPriority::MEDIUM;
		target.type = GameCamera::TargetType::MOVING;
		target.movingTarget = &box->getPosition();
		target.offset = Vec2f(0, -16);
		context.camera()->addTarget(target);

		drawPriority = 0;

		anim_idle     = Resources::get_animation_id("player", "idle");
		anim_idle_run = Resources::get_animation_id("player", "idle-running");
		anim_run      = Resources::get_animation_id("player", "running");
		anim_jump     = Resources::get_animation_id("player", "jump-air");
		anim_air      = Resources::get_animation_id("player", "air-fall");
		anim_fall     = Resources::get_animation_id("player", "fall");

		assert(anim_idle != AnimID::NONE);
		assert(anim_run  != AnimID::NONE);
		assert(anim_jump != AnimID::NONE);
		assert(anim_air  != AnimID::NONE);
		assert(anim_fall != AnimID::NONE);

		sprite.set_anim(anim_idle);

		sprite.set_pos(box->getPosition());
	};

	~Player() {
		if (context.valid()) {
			context.collision()->removeCollidable(box);
			context.camera()->removeTarget(GameCamera::TargetPriority::MEDIUM);
		}
	}

	std::unique_ptr<GameObject> clone() const override {

		std::unique_ptr<Player> object = std::make_unique<Player>(context, *getObjectRef());

		//TODO copy current state data

		return object;
	}

	void update(secs deltaTime) {

		int wishx = (int)Input::isHeld(InputType::RIGHT) - (int)Input::isHeld(InputType::LEFT);

		sprite.set_playback(1.f);

		
		// on ground
		if (ground.has_contact()) {
			const PersistantContact& contact = ground.get_contact().value();
			
			Vec2f groundUnit = contact.collider_normal.righthand();
			ground.slope_sticking = true;

			float speed = ground.traverse_get_speed();
			const static float max_speed = 500.f;


			if (ground.get_duration() == 0.0) {
				ground.traverse_set_max_speed(
					std::max(300.f, std::min(std::abs(speed), max_speed))
				);
			}
			else {
				ground.traverse_set_max_speed(
						std::max(300.f, std::min(std::abs(speed), ground.traverse_get_max_speed()))
					);
			}
			//LOG_INFO("{}", ground.traverse_get_max_speed());
			//ground.traverse_set_max_speed(300.f);


			//ground.traverse_set_max_speed(std::max(150.f, abs(speed)));

			int movex = (speed == 0.f ? 0 : (speed < 0.f ? -1 : 1));

			airtime = 0.0;

			// sinful
			//double target_rot = (90.f + math::angle(ground.get_contact()->collider_normal).degrees());
			//sprite.get_sprite().setRotation(target_rot / 3.f);


			if (box->get_vel().x == 0) {
				sprite.set_anim_if_not(anim_idle);
			}
			else {
				if (sprite.is_playing(anim_idle)) {
					sprite.set_anim(anim_idle_run);
				}

				sprite.set_anim_if_not(anim_run);

				sprite.set_playback(
					std::max(
						0.5f,
						std::abs(box->get_vel().magnitude()) / 250.f
					));
			}

			// jumping
			if (Input::isPressed(InputType::JUMP, 0.1f)) {
				Input::confirmPress(InputType::JUMP);

				sprite.set_anim(anim_jump);

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
					ground.traverse_add_accel(wishx * 600.f);

					bool isBraking = (movex != 0) && (wishx < 0) != (movex < 0);
					ground.surface_friction = isBraking ? braking : moving;

				}
				else if (ground.has_friction) {
					ground.traverse_add_decel(300.f);
					ground.surface_friction = braking;
				}
			}
		}
		// in air
		else {
			//sprite.get_sprite().setRotation(0);

			airtime += deltaTime;

			// flight control
			int wishy = (int)Input::isHeld(InputType::DOWN) - (int)Input::isHeld(InputType::UP);
			box->add_accelY(600.f * wishy);

			if (airtime > 0.05
				&& !sprite.is_playing(anim_jump)
				&& !sprite.is_playing(anim_air) 
				&& !sprite.is_playing(anim_fall)) 
			{
				sprite.set_anim(anim_air);
			}
			else if (sprite.is_complete(anim_air) && box->get_vel().y > 0) {

				sprite.set_anim(anim_fall);
			}

			ground.slope_sticking = false;
			// air control
			if (wishx != 0 && (abs(box->get_vel().x) < 150.f ||
				box->get_vel().x < 0.f != wishx < 0.f)) {
				box->add_accelX(600.f * wishx);
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

	void predraw(secs deltaTime) {
		sprite.set_pos(box->getPosition());
		sprite.predraw(deltaTime);
	}

	virtual void ImGui_Inspect() {
		ImGui::Text("Hello World!");
		ImGui::Text("Position(%3.2f, %3.2f)", box->getPosition().x, box->getPosition().y);
		ImGui::Text("Velocity(%3.2f, %3.2f)", box->get_vel().x, box->get_vel().y);

		//ImGui::Text("Braking: %s", (isBraking ? "true" : "false"));

	};

protected:

	secs airtime = 0.0;

	static AnimID anim_idle;
	static AnimID anim_idle_run;
	static AnimID anim_run;
	static AnimID anim_jump;
	static AnimID anim_air;
	static AnimID anim_fall;

	AnimatedSprite sprite;

	Friction braking;
	Friction moving;
	//bool isBraking = true;

	SurfaceTracker ground;

	float airNeutralVel;

	Collidable* box;

	void draw(RenderTarget& target, RenderState states = RenderState()) const override {
		target.draw(sprite, states);
	}
};

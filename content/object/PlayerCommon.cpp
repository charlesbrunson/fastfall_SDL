#include "PlayerCommon.hpp"

#include "fastfall/engine/input/InputConfig.hpp"

using namespace ff;

plr::move_t::move_t(World& w, const plr::members& plr)
{
    auto& sprite = w.at(plr.sprite_id);
    auto& box    = w.at(plr.collidable_id);
    auto& ground = *box.tracker();

	wishx = 0;
    if (w.input()[InputType::RIGHT].is_held()) wishx++;
    if (w.input()[InputType::LEFT].is_held()) wishx--;

	int flipper = (sprite.get_hflip() ? -1 : 1);

	auto gspeed = ground.traverse_get_speed();
	speed       = gspeed ? *gspeed : box.get_local_vel().x;
	rel_speed   = speed * flipper;

	movex       = (speed == 0.f ? 0 : (speed < 0.f ? -1 : 1));
	speed       = abs(speed);

	rel_movex   = movex * flipper;
	rel_wishx   = wishx * flipper;

	facing = flipper;
}


plr::members::members(ActorInit init, Vec2f position, bool face_dir)
    : sprite_id(        init.world.create<AnimatedSprite>(init.entity_id))
    , collidable_id(    init.world.create<Collidable>(init.entity_id, position, ff::Vec2f(8.f, 28.f), constants::grav_normal))
    , cameratarget_id()
    , hitbox_id(        init.world.create<Trigger>(init.entity_id, id_placeholder))
    , hurtbox_id(       init.world.create<Trigger>(init.entity_id, id_placeholder))
    , jet_id(      init.world.create<AnimatedSprite>(init.entity_id))
{
    World& w = init.world;
    auto& box = w.at(collidable_id);
    auto attachid = box.get_attach_id();
    box.create_tracker(
        Angle::Degree(-135.f),
        Angle::Degree( -45.f)
    );

    box.tracker()->settings = {
        .move_with_platforms = true,
        .slope_sticking      = true,
        .slope_wall_stop     = true,
        .has_friction        = true,
        .use_surf_vel        = true,
        .stick_angle_max = Angle::Degree(90.f),
        .max_speed = constants::norm_speed,
        .slope_stick_speed_factor = 0.f,
    };

    cameratarget_id = w.create<SimpleCamTarget>(
        init.entity_id,
        ff::CamTargetPriority::Medium,
        [id = collidable_id](World& w) {
            return w.at(id).getPosition() - Vec2f{0.f, 16.f};
        }
    );

    auto& hitbox = w.at(hitbox_id);
    hitbox.set_area(box.getBox());
    hitbox.self_flags = {"hitbox"};
    w.system<AttachSystem>().create(w, attachid, hitbox_id, { -box.getBox().width / 2.f, -box.getBox().height });

    auto& hurtbox = w.at(hurtbox_id);
    hurtbox.set_area(box.getBox());
    hurtbox.self_flags = {"hurtbox"};
    hurtbox.filter_flags = {"hitbox"};
    hurtbox.set_trigger_callback(
    [](World& w, const TriggerPull& pull) {
        // TODO
        /*
        auto ownerid = w.entity_of(pull.source->id());
        if (auto* obj = w.get(as_object(ownerid));
            obj && obj->type().group_tags.contains("player")
            && pull.state == Trigger::State::Entry)
        {
            objHurt.send(*obj, w, 100.f);
        }
        */
    });

    auto& sprite = w.at(sprite_id);
    sprite.set_anim(plr::anim::idle);
    sprite.set_hflip(face_dir);
    w.system<AttachSystem>().create(w, attachid, sprite_id);

    auto& jet_spr = w.at(jet_id);
    //jet_spr.visible = false;
    w.system<AttachSystem>().create(w, attachid, jet_id, Vec2f{ 0, -16 });

    auto& jetcfg = w.system<SceneSystem>().config(jet_id);
    jetcfg.visible = false;
    jetcfg.priority = scene_priority::Low;
}

namespace plr::anim {

	AnimIDRef idle("player.sax", "idle");
	AnimIDRef land("player.sax", "land");
    AnimIDRef land_soft("player.sax", "land_soft");
	AnimIDRef idle_to_run("player.sax", "idle_to_run");
	AnimIDRef run("player.sax", "running");

    /*
	AnimIDRef dash_n2("player.sax", "dash-2");
	AnimIDRef dash_n1("player.sax", "dash-1");
	AnimIDRef dash_0 ("player.sax", "dash0");
	AnimIDRef dash_p1("player.sax", "dash+1");
	AnimIDRef dash_p2("player.sax", "dash+2");
    */

	AnimIDRef jump("player.sax", "jump");
	AnimIDRef jump_f("player.sax", "jump_f");

	AnimIDRef fall("player.sax", "fall");
	AnimIDRef fall_f("player.sax", "fall_f");

	AnimIDRef brakeb("player.sax", "brake_back");
	AnimIDRef brakef("player.sax", "brake_front");

    dash_anim_t dash_n2{
        { "player.sax", "dash-2" },
        { "player.sax", "dash_fx-2" },
        { "player.sax", "jet_blast-2" },
        //{ 7.f, -12.f }
    };
    dash_anim_t dash_n1{
        { "player.sax", "dash-1" },
        { "player.sax", "dash_fx-1" },
        { "player.sax", "jet_blast-1" },
        //{ 6.f, -10.f }
    };
    dash_anim_t dash_0 {
        { "player.sax", "dash0" },
        { "player.sax", "dash_fx0"  },
        { "player.sax", "jet_blast0" },
        //{ 5.f, -15.f }
    };
    dash_anim_t dash_p1{
        { "player.sax", "dash+1" },
        { "player.sax", "dash_fx+1" },
        { "player.sax", "jet_blast+1" },
        //{ 0.f, -17.f }
    };
    dash_anim_t dash_p2{
        { "player.sax", "dash+2" },
        { "player.sax", "dash_fx+2" },
        { "player.sax", "jet_blast+2" },
        //{ 0.f, -18.f }
    };

	const std::vector<AnimID>& get_ground_anims() {
		static std::vector<AnimID> ground_anims{
			idle,
            land,
            run,
            brakeb,
            brakef,
			dash_n2.dash_anim,
            dash_n1.dash_anim,
            dash_0.dash_anim,
            dash_p1.dash_anim,
            dash_p2.dash_anim,
		};
		return ground_anims;
	}

	const std::vector<AnimID>& get_air_anims() {
		static std::vector<AnimID> air_anims{
			jump, jump_f,
			fall, fall_f
		};
		return air_anims;
	}
}

namespace plr::constants {

	Friction braking{ .stationary = 1.2f, .kinetic = 0.6f };
	Friction moving { .stationary = 0.0f, .kinetic = 0.0f };

	Default<float> max_speed = 400.f;
	Default<float> norm_speed = 150.f;
	Default<float> jumpVelY = -190.f;

	Default<float> ground_accel = 1000.f;
	Default<float> ground_high_decel = 200.f;
	Default<float> ground_idle_decel = 300.f;

	Default<Vec2f> grav_normal = Vec2f{ 0.f, 500.f };
	Default<Vec2f> grav_light = Vec2f{ 0.f, 400.f };


	Default<float> dash_speed = 220.f;
}

namespace plr::action {

	PlayerStateID dash(World& w, plr::members& plr, const move_t& move)
	{
        auto& sprite = w.at(plr.sprite_id);
		if (move.wishx != 0) {
			sprite.set_hflip(move.wishx < 0);
		}

        w.system<AudioSystem>().play("Bump.wav", audio::Volume{ 0.25f });
		return PlayerStateID::Dash;
	}

	PlayerStateID jump(World& w, plr::members& plr, const move_t& move)
	{
        auto& sprite = w.at(plr.sprite_id);
        auto& box = w.at(plr.collidable_id);
        auto& ground = *box.tracker();

		Vec2f contact_normal = Vec2f{0.f, -1.f};

		if (ground.has_contact()) {
			contact_normal = ground.get_contact()->collider_n;
		}

		float neutral_min_vx = -50.f;
		float neutral_max_vx = constants::norm_speed - 5.f;

		if (move.rel_speed > neutral_max_vx) {
			// running jump
			sprite.set_anim(anim::jump_f);
		}
		else if (move.rel_speed >= neutral_min_vx) {
			// neutral jump
			if (move.speed < 100.f && move.wishx != 0) {
				if (ground.has_contact()) {
					ground.traverse_set_speed(*ground.traverse_get_speed() + 50.f * move.wishx);
				}
				else {
					box.set_local_vel(box.get_local_vel().x + 50.f * move.wishx, {});
				}
			}
			sprite.set_anim(anim::jump);

		}
		else if (move.rel_speed < neutral_min_vx) {
			//back jump
			if (move.rel_wishx < 0) {
				sprite.set_hflip(!sprite.get_hflip());
				sprite.set_anim(anim::jump_f);
			}
			else {
				sprite.set_anim(anim::jump);
			}
		}

		ground.settings.slope_sticking = false;
		ground.settings.slope_wall_stop = false;

		Vec2f jumpVel = Vec2f{ box.get_local_vel().x, constants::jumpVelY };
		Angle jump_ang = math::angle(jumpVel) - math::angle(contact_normal);

		// from perpendicular to the ground
		static const Angle min_jump_ang = Angle::Degree(60);

		if (jump_ang < -min_jump_ang) {
			jumpVel = math::rotate(jumpVel, -jump_ang - min_jump_ang);
		}
		else if (jump_ang > min_jump_ang) {
			jumpVel = math::rotate(jumpVel, -jump_ang + min_jump_ang);
		}
		box.set_local_vel(jumpVel);

        w.system<AudioSystem>().play("Bump.wav", audio::Volume{ 0.5f });
		return PlayerStateID::Air;
	}
}

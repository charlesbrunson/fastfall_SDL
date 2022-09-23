#include "Player.hpp"
#include "PlayerCommon.hpp"

#include "fastfall/engine/input.hpp"
#include "fastfall/game/trigger/Trigger.hpp"


#include <functional>

using namespace ff;
using namespace plr;

const ObjectType Player::Type{
	.type = { "Player" },
	.allow_as_level_data = true,
	.anim = std::make_optional(AnimIDRef{"player.sax", "idle"}),
	.tile_size = { 1u, 2u },
	.group_tags = {	"player" },
	.properties = {
		{ "faceleft",	 false },
		{ "anotherprop", ObjectPropertyType::String }
	}
};

Player::Player(World& w, Vec2f position, bool faceleft)
	: GameObject{ w }
	, plr::members{ w, *this, position }
{
	init(w);
    auto& sprite = w.at_drawable<AnimatedSprite>(sprite_scene_id);
	sprite.set_hflip(faceleft);
};

Player::Player(World& w, ObjectLevelData& data)
	: GameObject( w, data )
	, plr::members{ w, *this, Vec2f{ data.position } }
{
	init(w);
    auto& sprite = w.at_drawable<AnimatedSprite>(sprite_scene_id);
	sprite.set_hflip(data.getPropAsBool("faceleft"));
};


void Player::init(World& w) {

    /*
	// trigger testing
	hurtbox->set_trigger_callback(
		[this](const TriggerPull& pull) 
		{
			if (auto owner = pull.trigger->get_owner();
				owner && owner->type().group_tags.contains("player")
				&& pull.state == Trigger::State::Entry)
			{
				if (auto& rpayload = owner->command<ObjCmd::GetPosition>().payload())
				{
					LOG_INFO("position: {}", rpayload->to_string());
					if (owner->command<ObjCmd::Hurt>(100.f)) 
					{
						LOG_INFO("gottem");
					}
				}
			}
		});
    */

    /*
    box->callbacks.onPostCollision = [this] {
			hitbox->set_area(box->getBox());
			manage_state(get_state().post_collision(*this));
		};
    */
}

void Player::manage_state(World& w, PlayerStateID n_id)
{
	auto state_transition = [this, &w]<std::derived_from<PlayerState> T>( T&& next_state )
	{
		get_state().exit(w, *this, &next_state);
		auto& prev_state = get_state();
		state = next_state;
		get_state().enter(w, *this, &prev_state);
	};

	switch (n_id) {
	case PlayerStateID::Continue:
		break;
	case PlayerStateID::Ground:
		state_transition(PlayerGroundState{});
		break;
	case PlayerStateID::Air:
		state_transition(PlayerAirState{});
		break;
	case PlayerStateID::Dash:
		state_transition(PlayerDashState{});
		break;
	}
}

void Player::update(World& w, secs deltaTime) {
	manage_state(w, get_state().update(w, *this, deltaTime));

    auto& sprite = w.at_drawable<AnimatedSprite>(sprite_scene_id);
	sprite.update(deltaTime);
}

/*
Player::CmdResponse Player::do_command(ObjCmd cmd, const std::any& payload) 
{
    return Behavior{ cmd, payload }
        .match<ObjCmd::NoOp>(		[]() { return true; })
        .match<ObjCmd::GetPosition>([this]() { return box->getPosition(); })
        .match<ObjCmd::Hurt>(		[this](float damage) { LOG_INFO("OUCH: {}", damage); });
}
*/

void Player::predraw(World& w, float interp, bool updated) {

    auto& sprite = w.at_drawable<AnimatedSprite>(sprite_scene_id);
    auto& box = w.at(collidable_id);
	sprite.set_pos(math::lerp(box.getPrevPosition(), box.getPosition(), interp));
	sprite.predraw(interp);
}


void Player::clean(ff::World& w) {
    w.erase(sprite_scene_id);
    w.erase(collidable_id);
    w.erase(hurtbox_id);
    w.erase(hitbox_id);
    w.erase(cameratarget_id);
}

/*
void Player::ImGui_Inspect() {
	using namespace ImGui;

	std::pair<PlayerStateID, std::string_view> pair = visit_state(
		[](PlayerState& state) { 
			return make_pair(state.get_id(), state.get_name());
		});
	auto id = pair.first;
	auto name = pair.second;

	ImGui::Text("Current State: %d %s", (int)id, name.data());
	ImGui::Text("Position(%3.2f, %3.2f)", box->getPosition().x, box->getPosition().y);
	ImGui::Text("Velocity(%3.2f, %3.2f)", box->get_vel().x, box->get_vel().y);

	Separator();

	DragFloat("Max Speed", &constants::max_speed.get(), 1.f, 0.f, 1000.f);
	SameLine(GetWindowWidth() - 60);
	if (SmallButton("reset##1")) { 
		constants::max_speed.restore();
	}

	if (DragFloat("Normal Speed", &constants::norm_speed.get(), 1.f, 0.f, 500.f)) {
		if (constants::norm_speed > ground->settings.max_speed) {
			ground->settings.max_speed = constants::norm_speed;
		}
	}
	SameLine(GetWindowWidth() - 60);
	if (SmallButton("reset##2")) {
		constants::norm_speed.restore();
	}

	DragFloat("Jump Vel", &constants::jumpVelY.get(), 1.f, -1000.f, 0.f);
	SameLine(GetWindowWidth() - 60);
	if (SmallButton("reset##3")) { 
		constants::jumpVelY.restore();
	}

	DragFloat("Dash Speed", &constants::dash_speed.get(), 10.f, 0.f, 5000.f);
	SameLine(GetWindowWidth() - 60);
	if (SmallButton("reset##3")) {
		constants::dash_speed.restore();
	}

	Separator();

	get_state().get_imgui(*this);
}
*/
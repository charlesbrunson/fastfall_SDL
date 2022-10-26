#include "Player.hpp"
#include "PlayerCommon.hpp"

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

Player::Player(World& w, ID<GameObject> id, Vec2f position, bool faceleft)
	: GameObject{ w , id }
	, plr::members{ w, *this, position, faceleft}
{
    auto& box = w.at(collidable_id);
    box.callbacks.onPostCollision = [
            id = getID(),
            collidable_id = collidable_id,
            hitbox_id = hitbox_id
    ] (World& w)
    {
        auto& player = (Player&)w.at(id);
        auto& box = w.at(collidable_id);
        auto& hitbox = w.at(hitbox_id);
        hitbox.set_area(box.getBox());
        player.manage_state(w, player.get_state().post_collision(w, player));
    };
};

Player::Player(World& w, ID<GameObject> id, ObjectLevelData& data)
	: GameObject( w, id, data )
	, plr::members{ w, *this, Vec2f{ data.position }, data.getPropAsBool("faceleft")}
{
    auto& box = w.at(collidable_id);
    box.callbacks.onPostCollision = [
        plr_id = id_cast<Player>(getID()),
        col_id = collidable_id,
        hit_id = hitbox_id
    ] (World& w)
    {
        auto [plr, colbox, hitbox] = w.at(plr_id, col_id, hit_id);
        hitbox.set_area(colbox.getBox());
        plr.manage_state(w, plr.get_state().post_collision(w, plr));
    };
};


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
    w.at(sprite_id).update(deltaTime);
}

objcfg::dresult Player::message(World& w, const objcfg::dmessage& msg) {
    switch(msg) {
        case objNoOp: {
            //LOG_INFO("Nothing!");
            return objNoOp.accept();
        }
        case objGetPos: {
            return objGetPos.accept(w.at(collidable_id).getPosition());
        }
        case objSetPos: {
            auto [pos] = objSetPos.unwrap(msg);
            w.at(collidable_id).setPosition(pos);
            return objSetPos.accept();
        }
        case objHurt: {
            // float [damage] = objHurt.unwrap(msg);
            //LOG_INFO("OUCH: {}", objHurt.unwrap_as<float>(msg));
            return objHurt.accept();
        }
    }
    return objcfg::reject;
};

void Player::predraw(World& w, float interp, bool updated) {
    auto [spr, box] = w.at(sprite_id, collidable_id);
	spr.set_pos(math::lerp(box.getPrevPosition(), box.getPosition(), interp));
	spr.predraw(interp);
}


void Player::clean(ff::World& w) {
    w.erase(scene_id);
    w.erase(collidable_id);
    w.erase(cameratarget_id);
    w.erase(hitbox_id);
    w.erase(hurtbox_id);
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
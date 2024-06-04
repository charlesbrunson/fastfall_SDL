#pragma once

#include "fastfall/game/World.hpp"

#include "PlayerCommon.hpp"
#include "plr_states/PlayerGround.hpp"
#include "plr_states/PlayerAir.hpp"
#include "plr_states/PlayerDash.hpp"

#include <string_view>
#include <variant>

class Player : public ff::Actor, public plr::members {
private:
    static const std::string_view prop_facing;

public:
    const static ff::ActorType actor_type;

    Player(ff::ActorInit init, ff::Vec2f position, bool faceleft);
    Player(ff::ActorInit init, const ff::LevelObjectData& data);

    void update(ff::World& w, secs deltaTime) override;
    dresult message(ff::World&, const dmessage&) override;

    void ImGui_Inspect() override;

protected:
	std::variant<
		PlayerGroundState, 
		PlayerAirState, 
		PlayerDashState
	> state = PlayerGroundState{};

	template<typename Callable>
	requires std::is_invocable_v<Callable, PlayerState&>
	auto visit_state(Callable&& callable)
	{
		return std::visit(callable, state);
	}

	PlayerState& get_state();

	void manage_state(ff::World& w, PlayerStateID n_id);
};

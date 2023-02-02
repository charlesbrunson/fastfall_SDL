#pragma once

#include "fastfall/game/object/Object.hpp"
#include "fastfall/game/World.hpp"

#include "PlayerCommon.hpp"
#include "plr_states/PlayerGround.hpp"
#include "plr_states/PlayerAir.hpp"
#include "plr_states/PlayerDash.hpp"

#include <string_view>
#include <variant>

class Player : public ff::Object, public plr::members {
public:
	static const ff::ObjectType Type;
	const ff::ObjectType& type() const override { return Type; };

	Player(ff::ActorInit init, ff::Vec2f position, bool faceleft);

	Player(ff::ActorInit init, ff::ObjectLevelData& data);

	void update(ff::World& w, secs deltaTime) override;

	//void ImGui_Inspect() override;

    dresult message(ff::World&, const dmessage&) override;

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

	PlayerState& get_state() {
		return std::visit([](auto& t_state) -> PlayerState& { 
			static_assert(std::derived_from<std::decay_t<decltype(t_state)>, PlayerState>, "all state types must be derived from PlayerState!");
			return static_cast<PlayerState&>(t_state); 
		}, state);
	}

	void manage_state(ff::World& w, PlayerStateID n_id);
};

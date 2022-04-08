#pragma once

#include "fastfall/game/object/GameObject.hpp"

#include "PlayerCommon.hpp"
#include "plr_states/PlayerGround.hpp"
#include "plr_states/PlayerAir.hpp"
#include "plr_states/PlayerDash.hpp"

#include <string_view>
#include <variant>

class Player : public ff::GameObject, public plr::members {
public:
	static const ff::ObjectType Type;
	const ff::ObjectType& type() const override { return Type; };

	Player(ff::GameContext context, ff::Vec2f position, bool faceleft);

	Player(ff::GameContext context, ff::ObjectLevelData& data);

	std::unique_ptr<ff::GameObject> clone() const override;

	void update(secs deltaTime) override;
	void predraw(float interp, bool updated) override;

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

	PlayerState& get_state() {
		return std::visit([](auto& t_state) -> PlayerState& { 
			static_assert(std::derived_from<std::decay_t<decltype(t_state)>, PlayerState>, "all state types must be derived from PlayerState!");
			return static_cast<PlayerState&>(t_state); 
		}, state);
	}

	void manage_state(PlayerStateID n_id);

	CmdResponse do_command(ff::ObjCmd cmd, const std::any& payload) override;

	void init();
};

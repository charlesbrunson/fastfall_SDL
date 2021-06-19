#pragma once

#include <assert.h>
#include <array>
#include <chrono>

#include "fastfall/engine/state/EngineState.hpp"
#include "fastfall/engine/imgui/ImGuiContent.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/game/Instance.hpp"

class TestState : public ff::EngineState {
public:
	TestState();
	~TestState();

	void update(secs deltaTime) override;
	void predraw(secs deltaTime) override;


	//void updateImGUI();

	inline void setEngineAction(const ff::EngineStateAction& act) noexcept { eAct = act; };

private:
	void draw(ff::RenderTarget& target, ff::RenderState states = ff::RenderState()) const override;

	ff::GameInstance* instance;

};

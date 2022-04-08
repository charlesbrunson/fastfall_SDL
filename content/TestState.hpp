#pragma once

#include <assert.h>
#include <array>
#include <chrono>

#include "fastfall/engine/state/EngineState.hpp"
#include "fastfall/engine/imgui/ImGuiContent.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/game/Instance.hpp"

#include "fastfall/game/level/LevelEditor.hpp"

#include "fastfall/render/Sprite.hpp"

class TestState : public ff::EngineState {
public:
	TestState();
	~TestState();

	void update(secs deltaTime) override;
	void predraw(float interp, bool updated) override;


	//void updateImGUI();

	inline void setEngineAction(const ff::EngineStateAction& act) noexcept { eAct = act; };

private:
	void draw(ff::RenderTarget& target, ff::RenderState states = ff::RenderState()) const override;

	ff::GameInstance* instance;

	bool painting = false;
	ff::Vec2i last_paint;

	std::unique_ptr<ff::LevelEditor> edit;

	std::unique_ptr<Uint8[]> prevKeys;

	const Uint8* currKeys = nullptr;
	int key_count;

	ff::Sprite tile_ghost;
	ff::TileLayer::world_pos_t ghost_pos;

	int layer = -1;

	ff::Vec2f mirror;
	ff::Vec2i tpos;
	ff::Vec2f mpos;

	ff::Text tile_text;
};

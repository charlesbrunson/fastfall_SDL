#pragma once

#include "fastfall/engine/state/EngineState.hpp"


class EmptyState : public ff::EngineState {
public:

	EmptyState() {
		clearColor = ff::Color(0x141013FF);
		viewPos = ff::Vec2f{100.f, 100.f};
	}

	void update(secs deltaTime) override {};
	void predraw(secs deltaTime) override {};

private:

	void draw(ff::RenderTarget& target, ff::RenderState state) const override {};
};
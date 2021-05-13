#pragma once

#include "fastfall/engine/state/EngineState.hpp"


class EmptyState : public ff::EngineState {
public:

	EmptyState() {
		clearColor = ff::Color(0x141013FF);
	}

	void update(secs deltaTime) override {};
	void predraw(secs deltaTime) override {};

private:

	void draw(ff::RenderTarget& target, ff::RenderState state) const override {};
};
#pragma once

#include "fastfall/engine/state/EngineState.hpp"


class EmptyState : public ff::EngineState {
public:

	void update(secs deltaTime) override {};
	void predraw(secs deltaTime) override {};

private:

	void draw(ff::RenderTarget& target, ff::RenderState state) const override {};
};
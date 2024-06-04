#pragma once

#include "fastfall/render/util/RenderState.hpp"
#include "fastfall/render/target/RenderTarget.hpp"
#include "fastfall/engine/time/time.hpp"

namespace ff {

//class RenderTarget;


class Drawable {
public:
	virtual ~Drawable() = default;

    virtual void update(secs deltaTime) {};
    virtual void predraw(predraw_state_t predraw_state) {};

	bool visible = true;

private:
	friend class RenderTarget;

	virtual void draw(RenderTarget& target, RenderState state = RenderState()) const = 0;

};

}
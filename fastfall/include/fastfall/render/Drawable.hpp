#pragma once

#include "RenderState.hpp"
#include "RenderTarget.hpp"

namespace ff {

//class RenderTarget;

class Drawable {
public:
	virtual ~Drawable() = default;

    virtual void predraw(float interp, bool updated) {};

	bool visible = true;

private:
	friend class RenderTarget;

	virtual void draw(RenderTarget& target, RenderState state = RenderState()) const = 0;

};

}
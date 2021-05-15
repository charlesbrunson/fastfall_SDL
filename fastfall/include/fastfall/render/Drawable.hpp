#pragma once

#include "RenderState.hpp"
#include "RenderTarget.hpp"

namespace ff {

//class RenderTarget;

class Drawable {
public:
	virtual ~Drawable() = default;

private:
	friend class RenderTarget;

	virtual void draw(RenderTarget& target, RenderState state) const = 0;

};

}
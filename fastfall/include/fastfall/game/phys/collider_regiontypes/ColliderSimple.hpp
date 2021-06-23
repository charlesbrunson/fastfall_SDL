
#pragma once

#include "fastfall/game/phys/ColliderRegion.hpp"
#include "fastfall/util/math.hpp"

// collider region containing a single collider

namespace ff {

class ColliderSimple : public ColliderRegion {
public:
	ColliderSimple(Rectf shape) :
		ColliderRegion{},
		quad(shape)
	{

		boundingBox = shape;

		quad.setID(0u);

	}

	void update(secs deltaTime) override {
		debugDrawQuad(quad, getPosition(), this);
	}
	const ColliderQuad* get_quad(int quad_id) const noexcept override {
		return quad_id == 0u ? &quad : nullptr;
	}

	void getQuads(Rectf area, std::vector<std::pair<Rectf, const ColliderQuad*>>& buffer) const override {
		//std::vector<std::pair<Rectf, const ColliderQuad*>> r;

		Rectf box = boundingBox;
		box.left += getPosition().x;
		box.top += getPosition().y;


		// Compute the min and max of the first rectangle on both axes
		float r1MinX = std::min(box.left, (box.left + box.width));
		float r1MaxX = std::max(box.left, (box.left + box.width));
		float r1MinY = std::min(box.top, (box.top + box.height));
		float r1MaxY = std::max(box.top, (box.top + box.height));

		// Compute the min and max of the second rectangle on both axes
		float r2MinX = std::min(area.left, (area.left + area.width));
		float r2MaxX = std::max(area.left, (area.left + area.width));
		float r2MinY = std::min(area.top, (area.top + area.height));
		float r2MaxY = std::max(area.top, (area.top + area.height));

		// Compute the intersection boundaries
		float interLeft = std::max(r1MinX, r2MinX);
		float interTop = std::max(r1MinY, r2MinY);
		float interRight = std::min(r1MaxX, r2MaxX);
		float interBottom = std::min(r1MaxY, r2MaxY);

		if ((interLeft <= interRight) && (interTop <= interBottom)) {
			buffer.push_back(std::make_pair(box, &quad));
		}
		//return r;
	}

private:

	ColliderQuad quad;

};

}
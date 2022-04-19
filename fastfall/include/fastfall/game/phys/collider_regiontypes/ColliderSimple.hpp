
#pragma once

#include "fastfall/game/phys/ColliderRegion.hpp"
#include "fastfall/util/math.hpp"

#include <functional>

// collider region containing a single collider

namespace ff {

class ColliderSimple : public ColliderRegion {
public:
	ColliderSimple(Rectf shape) :
		ColliderRegion{},
		quad(shape)
	{

		boundingBox = shape;
		prevBoundingBox = shape;

		quad.setID(0u);

	}

	void update(secs deltaTime) override {
		debugDrawQuad(quad, getPosition(), this);
	}
	const ColliderQuad* get_quad(int quad_id) const noexcept override {
		return quad_id == 0u ? &quad : nullptr;
	}

	void getQuads(Rectf area, std::vector<std::pair<Rectf, const ColliderQuad*>>& buffer) const override {
		
		Rectf bbox = area;
		bbox.left -= getPosition().x;
		bbox.top -= getPosition().y;

		Vec2f deltap = getPosition() - getPrevPosition();
		if (deltap.x < 0.f) {
			bbox = math::rect_extend(bbox, Cardinal::E, abs(deltap.x));
		}
		else if (deltap.x > 0.f) {
			bbox = math::rect_extend(bbox, Cardinal::W, abs(deltap.x));
		}

		if (deltap.y < 0.f) {
			bbox = math::rect_extend(bbox, Cardinal::S, abs(deltap.y));
		}
		else if (deltap.y > 0.f) {
			bbox = math::rect_extend(bbox, Cardinal::N, abs(deltap.y));
		}
		
		// Compute the min and max of the first rectangle on both axes
		float r1MinX = std::min(boundingBox.left, (boundingBox.left + boundingBox.width));
		float r1MaxX = std::max(boundingBox.left, (boundingBox.left + boundingBox.width));
		float r1MinY = std::min(boundingBox.top, (boundingBox.top + boundingBox.height));
		float r1MaxY = std::max(boundingBox.top, (boundingBox.top + boundingBox.height));

		// Compute the min and max of the second rectangle on both axes
		float r2MinX = std::min(bbox.left, (bbox.left + bbox.width));
		float r2MaxX = std::max(bbox.left, (bbox.left + bbox.width));
		float r2MinY = std::min(bbox.top, (bbox.top + bbox.height));
		float r2MaxY = std::max(bbox.top, (bbox.top + bbox.height));

		// Compute the intersection boundaries
		float interLeft = std::max(r1MinX, r2MinX);
		float interTop = std::max(r1MinY, r2MinY);
		float interRight = std::min(r1MaxX, r2MaxX);
		float interBottom = std::min(r1MaxY, r2MaxY);

		if ((interLeft <= interRight) && (interTop <= interBottom)) {
			buffer.push_back(std::make_pair(boundingBox, &quad));
		}
		//return r;
	}

	void set_on_precontact(std::function<bool(const Contact&, secs)> func) {
		callback_on_precontact = func;
	}
	void set_on_postcontact(std::function<void(const PersistantContact&)> func) {
		callback_on_postcontact = func;
	}

	bool on_precontact(int quad_id, const Contact& contact, secs duration) const override {
		if (callback_on_precontact)
			return callback_on_precontact(contact, duration);

		return true;
	}
	void on_postcontact(int quad_id, const PersistantContact& contact) const override {
		if (callback_on_postcontact)
			callback_on_postcontact(contact);
	}

private:

	std::function<bool(const Contact&, secs)> callback_on_precontact;
	std::function<void(const PersistantContact&)> callback_on_postcontact;

	ColliderQuad quad;

};

}
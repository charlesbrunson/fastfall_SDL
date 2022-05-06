
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
		
		Rectf bbox = math::shift(area, -getPosition());

		Vec2f deltap = getPosition() - getPrevPosition();
		bbox = math::rect_extend(bbox, (deltap.x < 0.f ? Cardinal::W : Cardinal::E), abs(deltap.x));
		bbox = math::rect_extend(bbox, (deltap.y < 0.f ? Cardinal::N : Cardinal::S), abs(deltap.y));

		if (boundingBox.touches(bbox)) {
			buffer.push_back(std::make_pair(boundingBox, &quad));
		}
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

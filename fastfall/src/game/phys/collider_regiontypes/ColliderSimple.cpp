#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"


namespace ff {

	ColliderSimple::ColliderSimple(Rectf shape) :
		ColliderRegion{},
		quad(shape)
	{
		boundingBox = shape;
		prevBoundingBox = shape;
		quad.setID({ 0u });
	}

	void ColliderSimple::update(secs deltaTime) {
		debugDrawQuad(quad, getPosition(), this);
	}
	const ColliderQuad* ColliderSimple::get_quad(QuadID quad_id) const noexcept {
		return quad_id.value == 0u ? &quad : nullptr;
	}

	void ColliderSimple::get_quads_in_rect(Rectf area, std::vector<std::pair<Rectf, QuadID>>& out_buffer) const {

		Rectf bbox = math::shift(area, -getPosition());

		Vec2f deltap = getPosition() - getPrevPosition();
		bbox = math::rect_extend(bbox, (deltap.x < 0.f ? Cardinal::W : Cardinal::E), abs(deltap.x));
		bbox = math::rect_extend(bbox, (deltap.y < 0.f ? Cardinal::N : Cardinal::S), abs(deltap.y));

		if (boundingBox.touches(bbox)) {
			out_buffer.push_back(std::make_pair(boundingBox, quad.getID()));
		}
	}

	void ColliderSimple::set_on_precontact(std::function<bool(World&, const ContinuousContact&, secs)> func) {
		callback_on_precontact = func;
	}
	void ColliderSimple::set_on_postcontact(std::function<void(World&, const AppliedContact&)> func) {
		callback_on_postcontact = func;
	}

	bool ColliderSimple::on_precontact(World& w, const ContinuousContact& contact, secs duration) const {
		if (callback_on_precontact)
			return callback_on_precontact(w, contact, duration);

		return true;
	}
	void ColliderSimple::on_postcontact(World& w, const AppliedContact& contact) const {
		if (callback_on_postcontact)
			callback_on_postcontact(w, contact);
	}
}

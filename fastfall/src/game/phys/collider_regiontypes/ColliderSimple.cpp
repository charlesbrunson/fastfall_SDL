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

    std::optional<QuadID> ColliderSimple::first_quad_in_rect(Rectf area, Recti& tile_area) const {
        Rectf bbox = math::shift(area, -getPosition());
        return boundingBox.touches(bbox) ? std::make_optional(quad.getID()) : std::nullopt;
    }
    std::optional<QuadID> ColliderSimple::next_quad_in_rect(Rectf area, QuadID quadid, const Recti& tile_area) const {
        // there's only one
        return {};
    }
    std::optional<QuadID> ColliderSimple::first_quad_in_line(Linef line, Recti& tile_area) const {
        bool contained = boundingBox.contains(math::shift(line, -getPosition()));
        return contained ? std::make_optional(quad.getID()) : std::nullopt;
    }
    std::optional<QuadID> ColliderSimple::next_quad_in_line(Linef line, QuadID quadid, const Recti& tile_area) const {
        // there's only one
        return {};
    }

	void ColliderSimple::set_on_precontact(std::function<bool(World&, const ContinuousContact&, secs)> func) {
		callback_on_precontact = func;
	}
	void ColliderSimple::set_on_postcontact(std::function<void(World&, const AppliedContact&, secs)> func) {
		callback_on_postcontact = func;
	}

	bool ColliderSimple::on_precontact(World& w, const ContinuousContact& contact, secs duration) const {
		if (callback_on_precontact)
			return callback_on_precontact(w, contact, duration);

		return true;
	}
	void ColliderSimple::on_postcontact(World& w, const AppliedContact& contact, secs deltaTime) const {
		if (callback_on_postcontact)
			callback_on_postcontact(w, contact, deltaTime);
	}

}

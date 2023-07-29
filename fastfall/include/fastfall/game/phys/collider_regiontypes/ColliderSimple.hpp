
#pragma once

#include "fastfall/game/phys/ColliderRegion.hpp"
#include "fastfall/util/math.hpp"

#include <functional>

// collider region containing a single collider

namespace ff {

class ColliderSimple : public ColliderRegion {
public:
	ColliderSimple(Rectf shape);

	void update(secs deltaTime) override;
	const ColliderQuad* get_quad(QuadID quad_id) const noexcept override;

	void set_on_precontact(std::function<bool(World&, const ContinuousContact&, secs)> func);
	void set_on_postcontact(std::function<void(World&, const AppliedContact&, secs)> func);

	bool on_precontact(World& w, const ContinuousContact& contact, secs duration) const override;
	void on_postcontact(World& w, const AppliedContact& contact, secs deltaTime) const override;

protected:
    std::optional<QuadID> first_quad_in_rect(Rectf area, Recti& tile_area) const override;
    std::optional<QuadID> next_quad_in_rect(Rectf area, QuadID quadid, const Recti& tile_area) const override;

private:
	std::function<bool(World&, const ContinuousContact&, secs)> callback_on_precontact;
	std::function<void(World&, const AppliedContact&, secs)> callback_on_postcontact;

	ColliderQuad quad;
};

}

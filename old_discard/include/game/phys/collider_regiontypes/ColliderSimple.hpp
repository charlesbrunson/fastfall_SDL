
#pragma once

#include "../ColliderRegion.hpp"
#include "fastfall/util/math.hpp"

#include <functional>

// collider region containing a single collider

namespace ff {

class ColliderSimple : public ColliderRegion {
public:
	ColliderSimple(Rectf shape);

	void update(secs deltaTime) override;
	const ColliderQuad* get_quad(int quad_id) const noexcept override;

	void get_quads_in_rect(Rectf area, std::vector<std::pair<Rectf, const ColliderQuad*>>& out_buffer) const override;

	void set_on_precontact(std::function<bool(const Contact&, secs)> func);
	void set_on_postcontact(std::function<void(const PersistantContact&)> func);

	bool on_precontact(int quad_id, const Contact& contact, secs duration) const override;
	void on_postcontact(const PersistantContact& contact) const override;

private:
	std::function<bool(const Contact&, secs)> callback_on_precontact;
	std::function<void(const PersistantContact&)> callback_on_postcontact;

	ColliderQuad quad;
};

}

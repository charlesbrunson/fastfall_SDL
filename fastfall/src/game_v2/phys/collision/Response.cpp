
#include "fastfall/game_v2/phys/collision/Response.hpp"

#include <tuple>

namespace ff {

namespace phys_resp {
	namespace {
		using NormalTangent = std::pair<Vec2f, Vec2f>;

		NormalTangent resp_standard(Vec2f curr_vel, const AppliedContact& contact) {

			Vec2f normal = math::projection(contact.velocity, contact.collider_n, true);
			Vec2f tangent = math::projection(curr_vel, contact.collider_n.righthand(), true);

			return std::make_pair(normal, tangent);
		}

		NormalTangent resp_flatten(Vec2f curr_vel, const AppliedContact& contact) {

			Vec2f normal = math::projection(contact.velocity, contact.ortho_n, true);
			Vec2f tangent = math::projection(curr_vel, contact.ortho_n.righthand(), true);
			return std::make_pair(normal, tangent);

		}
	}

	Vec2f get(const Collidable& body, const AppliedContact& contact, phys_resp::type response_type) {

		NormalTangent(*response)(Vec2f, const AppliedContact&);

		switch (response_type) {
		case type::STANDARD: response = resp_standard; break;
		case type::FLATTEN: response = resp_flatten; break;
		default: return body.get_vel();
		}

		auto [normal, tangent] = response(body.get_vel(), contact);

		return normal + tangent;
	}

}

}
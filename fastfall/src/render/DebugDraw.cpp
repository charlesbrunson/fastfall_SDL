#include "fastfall/render/DebugDraw.hpp"


namespace ff {

using DebugDrawable = std::pair<Vec2f, std::unique_ptr<Drawable>>;

std::vector<DebugDrawable> debugDrawListA;
std::vector<DebugDrawable> debugDrawListB;

std::vector<DebugDrawable>* activeList = &debugDrawListA;
std::vector<DebugDrawable>* inactiveList = &debugDrawListB;

Vec2f current_offset;

bool typeEnable[] = {
	true, // NONE
	true, // COLLISION_COLLIDER
	true, // COLLISION_COLLIDABLE
	true, // COLLISION_CONTACT
	true, // COLLISION_RAYCAST
};
constexpr unsigned typeEnableCount = (sizeof(typeEnable) / sizeof(typeEnable[0]));
static_assert(typeEnableCount == static_cast<unsigned>(debug_draw::Type::LAST), "debug draw type enum and type enable array count mismatch");

void debug_draw::setAllTypeEnabled(bool enable) {
	for (auto i = 0u; i != typeEnableCount; i++) {
		typeEnable[i] = enable;
	}
}

void debug_draw::setTypeEnabled(Type type, bool enable) {
	typeEnable[static_cast<unsigned>(type)] = enable;
}

bool debug_draw::hasTypeEnabled(Type type) {
	return typeEnable[static_cast<unsigned>(type)];
}

void debug_draw::add(std::unique_ptr<Drawable>&& drawable, Type type) {
	if (hasTypeEnabled(type))
		inactiveList->push_back(std::make_pair(current_offset, std::move(drawable)));
}

void debug_draw::swapDrawLists() {
	std::swap(activeList, inactiveList);
	inactiveList->clear();
}

void debug_draw::draw(RenderTarget& target, RenderState states) {
	for (auto& drawable : *activeList) {
		RenderState state = states;
		state.transform = Transform::combine(states.transform, Transform(drawable.first));
		if (drawable.second) {
			target.draw(*drawable.second.get(), state);
		}
	} 
}

void debug_draw::set_offset(Vec2f offset) {
	current_offset = offset;
}

}
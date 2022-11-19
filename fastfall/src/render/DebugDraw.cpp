#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/render/ShapeRectangle.hpp"

#include <set>
#include <deque>

namespace ff {

//using DebugDrawable = std::pair<Vec2f, std::unique_ptr<Drawable>>;

struct DebugDrawable {
	Vec2f offset;
	std::unique_ptr<Drawable> drawable;
	debug_draw::Type type;
	const void* signature = nullptr;
};

bool isEnabled = false;
bool isDark = false;

std::deque<DebugDrawable> debugDrawListA;
std::deque<DebugDrawable> debugDrawListB;

std::deque<DebugDrawable>* activeList = &debugDrawListA;
std::deque<DebugDrawable>* inactiveList = &debugDrawListB;

struct Repeat {
	const void* signature = nullptr;
	Vec2f offset;


	inline bool operator< (const Repeat& rhs) const {
		return signature < rhs.signature;
	}
	inline bool operator==(const Repeat& rhs) const {
		return signature == rhs.signature;
	}
};
std::set<Repeat> repeatList;

Vec2f current_offset;

bool typeEnable[] = {
	false,	// NONE
	true,	// COLLISION_COLLIDER
	true,	// COLLISION_COLLIDABLE
	true,	// COLLISION_CONTACT
	false,	// COLLISION_RAYCAST
	false,	// TILELAYER_AREA
	false,	// TILELAYER_CHUNK
	false,	// CAMERA_VISIBLE
	false,	// CAMERA_TARGET
	false,	// TRIGGER_AREA
    true,	// PATHS
    true,   // ATTACH
};
constexpr unsigned typeEnableCount = (sizeof(typeEnable) / sizeof(typeEnable[0]));
static_assert(typeEnableCount == static_cast<unsigned>(debug_draw::Type::LAST), "debug draw type enum and type enable array count mismatch");


void debug_draw::enable(bool enabled) {
	isEnabled = enabled;
}
bool debug_draw::hasEnabled() {
	return isEnabled;
}

void debug_draw::setAllTypeEnabled(bool enable) {
	for (auto i = 0u; i != typeEnableCount; i++) {
		typeEnable[i] = enable;
	}
	if (!enable) {
		activeList->clear();
		inactiveList->clear();
	}
}

void debug_draw::setTypeEnabled(Type type, bool enable) {
	typeEnable[static_cast<unsigned>(type)] = enable;
}

bool debug_draw::hasTypeEnabled(Type type) {
	return hasEnabled() && typeEnable[static_cast<unsigned>(type)];
}

void debug_draw::add(std::unique_ptr<Drawable>&& drawable, Type type, const void* signature) {
	if (hasTypeEnabled(type))
		inactiveList->push_back(DebugDrawable{ current_offset, std::move(drawable), type, signature });
}


bool debug_draw::is_darken() {
    return isDark;
}

void debug_draw::set_darken(bool dark) {
    isDark = dark;
}

void debug_draw::clear() {
	inactiveList->clear();
	activeList->clear();
	repeatList.clear();
}

bool debug_draw::repeat(const void* signature, Vec2f offset) {
	if (!signature)
		return false;

	auto iter = std::find_if(activeList->begin(), activeList->end(), [&signature](const DebugDrawable& debug) {
			return debug.signature == signature;
		});

	if (iter != activeList->end()) {
		repeatList.insert(Repeat{ signature, offset });
	}
	return (iter != activeList->end());

}

void debug_draw::swapDrawLists() {

    std::deque<DebugDrawable> tmp;

	for (auto& debug : *activeList) {
		if (debug.signature) {
			if (const auto it = repeatList.find(Repeat{ debug.signature });
				it != repeatList.end())
			{
				debug.offset = it->offset;
				inactiveList->push_front(std::move(debug));
			}
		}
	}
	activeList->clear();
	std::swap(activeList, inactiveList);
	repeatList.clear();

}

void debug_draw::draw(RenderTarget& target, RenderState states) {
    if (isDark)
    {
        ShapeRectangle sh{Rectf{ Vec2f{target.coordToWorldPos({})}, target.getSize() }, Color::Black().alpha(128) };
        target.draw(sh, states);
    }

	for (auto& debug : *activeList) {

		RenderState state = states;
		state.transform = Transform::combine(states.transform, Transform(debug.offset));
		if (debug.drawable) {
			target.draw(*debug.drawable.get(), state);
		}
	} 
}

void debug_draw::set_offset(Vec2f offset) {
	current_offset = offset;
}

}
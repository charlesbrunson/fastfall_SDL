#pragma once

//#include <SFML/Graphics.hpp>

#include "fastfall/util/math.hpp"
#include "fastfall/render/Drawable.hpp"
#include "fastfall/render/RenderTarget.hpp"
#include "fastfall/render/VertexArray.hpp"
#include "fastfall/render/Drawable.hpp"

#include <memory>

namespace ff {

namespace debug_draw {
	enum class Type {
		NONE,

		COLLISION_COLLIDER,
		COLLISION_COLLIDABLE,
		COLLISION_CONTACT,
		COLLISION_RAYCAST,

		TILELAYER_AREA,
		TILELAYER_CHUNK,

		CAMERA_VISIBLE,
		CAMERA_TARGET,

		TRIGGER_AREA,

		LAST
	};

	void enable(bool enabled = true);
	bool hasEnabled();

	void setAllTypeEnabled(bool enable);

	void setTypeEnabled(Type type, bool enable);
	bool hasTypeEnabled(Type type);

	void add(std::unique_ptr<Drawable>&& drawable, Type type = Type::NONE, const void* signature = nullptr);
	void swapDrawLists();

	void draw(RenderTarget& target, RenderState states = RenderState());

	void set_offset(Vec2f offset = Vec2f{});

	bool repeat(const void* signature, Vec2f offset);

	void clear();
};

// wrapper for declaring drawable for debugging
// T must be sf::Drawable subclass
template<class T, debug_draw::Type type = debug_draw::Type::NONE, class ...Args, typename = std::enable_if_t<std::is_base_of<Drawable, T>::value>>
T& createDebugDrawable(const void* sign, Args&&...args) {
	std::unique_ptr<Drawable> ptr = std::make_unique<T>(args...);
	T* t = static_cast<T*>(ptr.get());
	debug_draw::add(std::move(ptr), type, sign);
	return *t;
};

// wrapper for declaring drawable for debugging
// T must be sf::Drawable subclass
template<class T, debug_draw::Type type = debug_draw::Type::NONE, class ...Args, typename = std::enable_if_t<std::is_base_of<Drawable, T>::value>>
T& createDebugDrawable(Args&&...args) {
	std::unique_ptr<Drawable> ptr = std::make_unique<T>(args...);
	T* t = static_cast<T*>(ptr.get());
	debug_draw::add(std::move(ptr), type);
	return *t;
};

}
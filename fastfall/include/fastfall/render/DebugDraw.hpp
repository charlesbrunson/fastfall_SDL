#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/render/drawable/Drawable.hpp"
#include "fastfall/render/target/RenderTarget.hpp"
#include "fastfall/render/drawable/VertexArray.hpp"

#include <memory>
#include <array>
#include <span>

namespace ff {

namespace debug {
    namespace detail {
        struct draw_call_t {
            size_t id;
            size_t vertex_offset;
            size_t vertex_count;
            ff::Primitive primitive;
            Vec2f draw_offset;
        };

        struct state_t {
            std::vector<std::pair<const void*, size_t>> signatures;
            std::vector<draw_call_t> calls;
            std::vector<draw_call_t> compressed_calls;
            std::vector<Vertex> vertices;
            size_t repeat_calls = 0;

            void clear() {
                signatures.clear();
                calls.clear();
                compressed_calls.clear();
                vertices.clear();
                repeat_calls = 0;
            }
        };

        struct gpu_state_t {
            GLuint m_array   = 0;
            GLuint m_buffer  = 0;
            size_t m_bufsize = 0;

            bool m_bound = false;
            bool m_sync  = false;
        };
    }

    enum type {
        Collision_Collider,
        Collision_Collidable,
        Collision_Contact,
        Collision_Raycast,
        Collision_Tracker,
        Tilelayer_Area,
        Tilelayer_Chunk,
        Camera_Visible,
        Camera_Target,
        Trigger_Area,
        Path,
        Attach,
        Emitter,
    };

    inline constexpr static std::array types = {
        Collision_Collider,
        Collision_Collidable,
        Collision_Contact,
        Collision_Raycast,
        Collision_Tracker,
        Tilelayer_Area,
        Tilelayer_Chunk,
        Camera_Visible,
        Camera_Target,
        Trigger_Area,
        Path,
        Attach,
        Emitter,
    };
    std::string_view to_str(type t);


    void set(type t, bool set);
    void set_all(bool set);
    bool enabled(type t);

    bool type_enabled(type t);

    std::span<Vertex> draw(ff::Primitive primitive, size_t vertex_count, Vec2f offset = {});
    std::span<Vertex> draw(const void* signature, ff::Primitive primitive, size_t vertex_count, Vec2f offset = {});
    bool repeat(const void* signature, Vec2f offset = {});

    void reset();

    void predraw(bool updated);
    void draw(RenderTarget& target, RenderState states = RenderState());

    void init();
    void cleanup();

    struct stats_t {
        size_t draw_calls;
        size_t vertices;
        size_t repeat_calls;
    };

    extern bool show;
    extern bool darken;
    extern stats_t stats;
}


/*

namespace debug_draw {
	enum class Type {
		NONE,
		COLLISION_COLLIDER,
		COLLISION_COLLIDABLE,
		COLLISION_CONTACT,
		COLLISION_RAYCAST,
        COLLISION_TRACKER,
		TILELAYER_AREA,
		TILELAYER_CHUNK,
		CAMERA_VISIBLE,
		CAMERA_TARGET,
		TRIGGER_AREA,
        PATHS,
        ATTACH,
        EMITTER,
		LAST
	};

	void enable(bool enabled = true);
	bool hasEnabled();

	void setAllTypeEnabled(bool enable);

	void setTypeEnabled(Type type, bool enable);
	bool hasTypeEnabled(Type type);

    bool& type_state(Type type);

	void add(std::unique_ptr<Drawable>&& drawable, Type type = Type::NONE, const void* signature = nullptr);
	void swapDrawLists();

	void draw(RenderTarget& target, RenderState states = RenderState());

	void set_offset(Vec2f offset = Vec2f{});

	bool repeat(const void* signature, Vec2f offset);

    bool is_darken();
    void set_darken(bool dark = true);

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
*/

}
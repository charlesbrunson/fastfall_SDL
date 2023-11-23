#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/render/drawable/ShapeRectangle.hpp"
#include "detail/error.hpp"
#include "fastfall/render/render.hpp"

#include <set>
#include <deque>

namespace ff::debug {

    bool show = false;
    bool darken = false;
    stats_t stats;


    static constexpr std::array type_strs = {
            "Collision_Collider",
            "Collision_Collidable",
            "Collision_Contact",
            "Collision_Raycast",
            "Collision_Tracker",
            "Tilelayer_Area",
            "Tilelayer_Chunk",
            "Camera_Visible",
            "Camera_Target",
            "Trigger_Area",
            "Path",
            "Attach",
            "Emitter"
    };
    static_assert(type_strs.size() == types.size());

    inline static std::set<type> types_enabled = {
        Collision_Raycast
    };

    std::string_view to_str(type t) {
        return type_strs.at(static_cast<size_t>(t));
    }

    detail::state_t state_A;
    detail::state_t state_B;

    detail::state_t* curr_state = &state_A;
    detail::state_t* prev_state = &state_B;

    detail::gpu_state_t gl;

    void set(type t, bool set) {
        if (set)
            types_enabled.insert(t);
        else
            types_enabled.erase(t);
    }

    void set_all(bool set) {
        if (set)
            types_enabled.insert(types.begin(), types.end());
        else
            types_enabled.clear();
    }

    bool enabled(type t) {
        return show && types_enabled.contains(t);
    }

    bool type_enabled(type t) {
        return types_enabled.contains(t);
    }

    std::span<Vertex> draw(ff::Primitive primitive, size_t vertex_count, Vec2f offset) {

        auto* state   = curr_state;
        auto call_id  = state->calls.size();
        auto v_offset = state->vertices.size();

        state->calls.emplace_back(
            call_id,
            v_offset,
            vertex_count,
            primitive,
            offset
        );

        auto& cmpr = state->compressed_calls;
        if (!cmpr.empty()
            && (primitive == ff::Primitive::POINT || primitive == ff::Primitive::LINES || primitive == ff::Primitive::TRIANGLES )
            && cmpr.back().primitive   == primitive
            && cmpr.back().draw_offset == offset)
        {
            cmpr.back().vertex_count += vertex_count;
        }
        else {
            cmpr.emplace_back(state->calls.back());
        }

        auto it = state->vertices.insert(state->vertices.end(), vertex_count, {});
        return { it, vertex_count };
    }

    std::span<Vertex> draw(const void* signature, ff::Primitive primitive, size_t vertex_count, Vec2f offset) {
        auto* state = curr_state;
        auto pair = std::make_pair(signature, state->calls.size());
        state->signatures.insert(
            std::lower_bound(
                    state->signatures.begin(),
                    state->signatures.end(),
                    pair),
            pair
        );
        return draw(primitive, vertex_count, offset);
    }

    bool repeat(const void* signature, Vec2f offset) {
        if (!signature)
            return false;

        auto curr_it = std::lower_bound(
                curr_state->signatures.begin(),
                curr_state->signatures.end(),
                signature,
                [](const auto& pair, const void* sign) {
                    return pair.first < sign;
                }
        );

        if (std::find_if(
                curr_state->signatures.begin(),
                curr_state->signatures.end(),
                [&](const auto& pair) {
                    return pair.first == signature;
                }) != curr_state->signatures.end())
            return true;

        auto* state = prev_state;
        auto beg = std::lower_bound(
            state->signatures.begin(),
            state->signatures.end(),
            signature,
            [](const auto& pair, const void* sign) {
                return pair.first < sign;
            }
        );

        auto end = std::upper_bound(
            state->signatures.begin(),
            state->signatures.end(),
            signature,
            [](const void* sign, const auto& pair) {
                return sign < pair.first;
            }
        );

        bool repeat = false;
        for (; beg != end; ++beg) {
            const auto& call = state->calls[beg->second];
            auto span = draw(signature, call.primitive, call.vertex_count, offset);

            auto prev_beg = state->vertices.begin() + call.vertex_offset;
            auto prev_end = prev_beg + call.vertex_count;
            std::copy(prev_beg, prev_end, span.begin());
            ++curr_state->repeat_calls;

            repeat = true;
        }
        return repeat;
    }

    void reset() {
        curr_state->clear();
        prev_state->clear();
    }

    void predraw(bool updated) {
        if (updated) {
            stats = {
                .draw_calls = curr_state->compressed_calls.size(),
                .vertices   = curr_state->vertices.size(),
                .repeat_calls = curr_state->repeat_calls
            };

            std::swap(curr_state, prev_state);
            curr_state->clear();
            gl.m_sync = false;
        }
    }

    void draw(RenderTarget& target, RenderState states) {

        if (gl.m_array == 0) {

            // do the opengl initializaion
            glCheck(glGenVertexArrays(1, &gl.m_array));
            glCheck(glGenBuffers(1, &gl.m_buffer));

            glCheck(glBindVertexArray(gl.m_array));
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, gl.m_buffer));

            size_t position = 0lu;

            // position attribute
            glCheck(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ff::Vertex), (void*)position));
            glCheck(glEnableVertexAttribArray(0));
            position += (2 * sizeof(float));

            // color attribute
            glCheck(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ff::Vertex), (void*)position));
            glCheck(glEnableVertexAttribArray(1));
            position += sizeof(ff::Color);

            // tex attribute
            glCheck(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ff::Vertex), (void*)position));
            glCheck(glEnableVertexAttribArray(2));

            if (gl.m_array == 0 || gl.m_buffer == 0) {
                LOG_ERR_("Unable to initialize vertex array for opengl");
                assert(false);
            }
        }

        auto& state = *prev_state;
        if (!gl.m_sync && !state.vertices.empty()) {
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, gl.m_buffer));
            if (!gl.m_bound || state.vertices.size() > gl.m_bufsize) {
                glCheck(glBufferData(GL_ARRAY_BUFFER, state.vertices.size() * sizeof(Vertex), state.vertices.data(), GL_DYNAMIC_DRAW));
                gl.m_bufsize = state.vertices.size();
                gl.m_bound = true;
            }
            else {
                glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0 * sizeof(Vertex), state.vertices.size() * sizeof(Vertex), state.vertices.data()));
            }
            gl.m_sync = true;
        }

        if (darken)
        {
            ShapeRectangle sh{Rectf{ Vec2f{target.coordToWorldPos({})}, target.getSize() }, Color::Black().alpha(128) };
            target.draw(sh, states);
        }

        target.draw(*prev_state, gl);
    }

    void cleanup() {
        reset();
        glStaleVertexArrays(gl.m_array);
        glStaleVertexBuffers(gl.m_buffer);
    }
}



/*

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
	false,	// COLLISION_COLLIDER
	false,	// COLLISION_COLLIDABLE
	false,	// COLLISION_CONTACT
	true,	// COLLISION_RAYCAST
    false,	// COLLISION_TRACKER
	false,	// TILELAYER_AREA
	false,	// TILELAYER_CHUNK
	false,	// CAMERA_VISIBLE
	false,	// CAMERA_TARGET
	false,	// TRIGGER_AREA
    false,	// PATHS
    false,   // ATTACH
    false,   // EMITTER
};
constexpr unsigned typeEnableCount = (sizeof(typeEnable) / sizeof(typeEnable[0]));
static_assert(typeEnableCount == static_cast<unsigned>(debug_draw::Type::LAST), "debug draw type enum and type enable array count mismatch");



bool& debug_draw::type_state(Type type) {
    return typeEnable[static_cast<unsigned>(type)];
}

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
*/
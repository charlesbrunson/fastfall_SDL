#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/render/drawable/ShapeRectangle.hpp"
#include "detail/error.hpp"
#include "fastfall/render/render.hpp"

#include <set>
#include <deque>

namespace ff::debug {

    bool show = true;
    bool darken = false;
    stats_t stats;


    static constexpr std::array type_strs = {
            "Collision_Collider",
            "Collision_Collidable",
            "Collision_Contact",
            "Collision_Raycast",
            "Collision_Tracker",
            "Collision_Follow",
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
        Collision_Follow
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

    std::span<Vertex> draw(ff::Primitive primitive, size_t vertex_count, Vec2f offset, Vertex vertex) {

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

        auto it = state->vertices.insert(state->vertices.end(), vertex_count, vertex);
        return { it, vertex_count };
    }

    std::span<Vertex> draw(const void* signature, ff::Primitive primitive, size_t vertex_count, Vec2f offset, Vertex vertex) {
        auto* state = curr_state;
        auto pair = std::make_pair(signature, state->calls.size());
        state->signatures.insert(
            std::lower_bound(
                    state->signatures.begin(),
                    state->signatures.end(),
                    pair),
            pair
        );
        return draw(primitive, vertex_count, offset, vertex);
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

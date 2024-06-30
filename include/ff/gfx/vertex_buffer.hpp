#pragma once

#include "ff/util/math.hpp"
#include "ff/gfx/vertex.hpp"

#include <span>
#include <optional>

namespace ff {

enum class buffer_usage {
    Stream  = 0x88E0,
    Static  = 0x88E4,
    Dynamic = 0x88E8,
};

class vertex_buffer {
public:
    vertex_buffer();
    explicit vertex_buffer(buffer_usage t_usage);

    template<class VertexSpan>
    vertex_buffer(const VertexSpan& t_vertices) : vertex_buffer() {
        reset(t_vertices, m_usage);
    };

    template<class VertexSpan>
    vertex_buffer(const VertexSpan& t_vertices, buffer_usage t_usage) : vertex_buffer() {
        reset(t_vertices, t_usage);
    };

    vertex_buffer(const vertex_buffer&) = delete;
    vertex_buffer& operator=(const vertex_buffer&) = delete;

    vertex_buffer(vertex_buffer&& t_vertbuf) noexcept;
    vertex_buffer& operator=(vertex_buffer&& t_vertbuf) noexcept;
    ~vertex_buffer();

    void bind() const;

    bool reset(std::optional<buffer_usage> t_usage = {}) {
        m_usage = t_usage.value_or(m_usage);
        return copy_impl(nullptr, 0, 0, true);
    }

    inline bool reset(size_t t_size_bytes, std::optional<buffer_usage> t_usage = {}) {
        return copy_impl(nullptr, 0, t_size_bytes, true);
    }

    template<class VertexSpan>
    inline bool reset(const VertexSpan& t_vertices, std::optional<buffer_usage> t_usage = {}) {
        std::span v_span{ t_vertices };
        return copy_impl(v_span.data(), v_span.size_bytes(), 0, true);
    }

    template<class VertexSpan>
    inline bool copy(const VertexSpan& t_vertices, size_t t_offset) {
        std::span v_span{ t_vertices };
        return copy_impl(v_span.data(), v_span.size_bytes(), t_offset * sizeof(VertexSpan::value_type), false);
    }

private:
    bool copy_impl(const void* t_data, size_t t_size, size_t t_offset, bool reset);

    u32 m_id;
    size_t m_bufsize = 0;
    buffer_usage m_usage = buffer_usage::Static;
};

}
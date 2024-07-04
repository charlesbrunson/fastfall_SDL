#pragma once

#include "ff/util/math.hpp"

#include "vertex.hpp"
#include "gpu_buffer.hpp"

#include <vector>
#include <span>

namespace ff {


class vertex_array {
public:
    struct loc_attr {
        u32 loc;
        const vertex_attribute* attr;
    };

    vertex_array();

    vertex_array(const vertex_array&) = delete;
    vertex_array& operator=(const vertex_array&) = delete;

    vertex_array(vertex_array&& t_vertbuf) noexcept;
    vertex_array& operator=(vertex_array&& t_vertbuf) noexcept;

    ~vertex_array();

    void bind() const;
    inline u32 id() const { return m_id; }
    inline const auto attributes() const { return std::span{m_attributes}; };

    template<is_vertex V, class I>
    void assign_vertex_buffer(u32 start_loc, const gpu_buffer<V, I>& buf, u32 divisor = 0) {
        assign_vertex_buffer(start_loc, buf.subspan(), divisor);
    }

    template<is_vertex V, class I>
    void assign_vertex_buffer(u32 start_loc, gpu_span<V, I> buf, u32 divisor = 0) {
        impl_assign_vertex_buffer(start_loc, buf.id(), vertex_traits<V>::attributes, divisor);
    }

private:
    void impl_assign_vertex_buffer(u32 start_loc, u32 buf_id, std::span<const vertex_attribute> vattrs, u32 divisor);

    u32 m_id = 0;
    std::vector<loc_attr> m_attributes;
};

}
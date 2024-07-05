#pragma once

#include "ff/util/math.hpp"

#include "vertex.hpp"
#include "gpu_buffer.hpp"

#include <vector>
#include <span>
#include <optional>

namespace ff {


class vertex_array {
public:
    struct loc_attr {
        u32 loc;
        u32 count;
        u32 divisor;
        bool enabled = false;
        const vertex_attribute* attr;
    };
    struct elements_info {
        u32 index_type;
        u32 element_count;
        const void* offset;
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

    inline const std::optional<elements_info>& elements() const{
        return m_elements_info;
    }

    template<is_vertex V, class I>
    void assign_vertex_buffer(u32 start_loc, const gpu_buffer<V, I>& buf, u32 divisor = 0) {
        assign_vertex_buffer(start_loc, buf.subspan(), divisor);
    }

    template<is_vertex V, class I>
    void assign_vertex_buffer(u32 start_loc, gpu_span<V, I> buf, u32 divisor = 0) {
        impl_assign_vertex_buffer(start_loc, buf.id(), buf.offset_bytes(), buf.size(), vertex_traits<V>::attributes, divisor);
    }

    template<std::unsigned_integral V, class I>
    void assign_element_buffer(const gpu_buffer<V, I>& buf) {
        assign_element_buffer(buf.subspan());
    }

    template<std::unsigned_integral T, class I>
    void assign_element_buffer(gpu_span<T, I> buf) {
        impl_assign_element_buffer(buf.id(), buf.offset_bytes(), buf.size(), type_value_v<T>);
    }

    void unassign_element_buffer() {
        impl_assign_element_buffer(0, 0, 0, 0);
    }


private:
    void impl_assign_vertex_buffer(u32 start_loc, u32 buf_id, size_t buf_offset, u32 buf_size, std::span<const vertex_attribute> vattrs, u32 divisor);
    void impl_assign_element_buffer(u32 buf_id, size_t buf_offset, u32 buf_size, u32 index_type);

    u32 m_id = 0;
    std::vector<loc_attr> m_attributes;
    std::optional<elements_info> m_elements_info;
};

}
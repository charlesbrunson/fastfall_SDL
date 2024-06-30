#pragma once

#include "vertex_buffer.hpp"

#include "ff/util/math.hpp"

#include "vertex.hpp"
#include "vertex_buffer.hpp"
#include "element_buffer.hpp"

#include <vector>
#include <span>

namespace ff {

class vertex_array_base {
public:
    vertex_array_base(const vertex_array_base&) = delete;
    vertex_array_base& operator=(const vertex_array_base&) = delete;

    vertex_array_base(vertex_array_base&& t_vertbuf) noexcept;
    vertex_array_base& operator=(vertex_array_base&& t_vertbuf) noexcept;
    ~vertex_array_base();

    [[nodiscard]] u32 id() const { return m_id; }
    [[nodiscard]] auto attributes() const { return std::span{ m_attributes }; }
    [[nodiscard]] const vertex_buffer* vertex_buffer_ptr() const { return m_vertbuf; }
    [[nodiscard]] const element_buffer* element_buffer_ptr() const { return m_elembuf; }

    void bind() const;

protected:
    vertex_array_base(const std::span<const attribute_info> t_attrs, vertex_buffer* t_vertbuf, element_buffer* t_elembuf);

private:
    u32 m_id = 0;
    std::vector<attribute_info> m_attributes;
    vertex_buffer*  m_vertbuf = nullptr;
    element_buffer* m_elembuf = nullptr;
};

template<is_vertex Vertex>
class vertex_array : public vertex_array_base {
public:
    using vertex_type = Vertex;
    constexpr static size_t component_count = Vertex::attributes::size;

    // assume vertex data is interleaved
    explicit vertex_array(vertex_buffer& t_vertbuf)
    : vertex_array_base( attribute_info_array_v<Vertex>, &t_vertbuf, nullptr )
    {
    }

    vertex_array(vertex_buffer& t_vertbuf, element_buffer& t_elembuf)
    : vertex_array_base( attribute_info_array_v<Vertex>, &t_vertbuf, &t_elembuf )
    {
    }
};

}
#include "ff/gfx/vertex_array.hpp"

#include "../external/glew.hpp"

namespace ff {

vertex_array_base::vertex_array_base(
    const std::span<const attribute_info> t_attrs,
    vertex_buffer*  t_vertbuf,
    element_buffer* t_elembuf)
: m_id{}
, m_attributes{ t_attrs.begin(), t_attrs.end() }
, m_vertbuf{ t_vertbuf }
, m_elembuf{ t_elembuf }
{
    glCheck(glGenVertexArrays(1, &m_id));
    bind();

    if (t_vertbuf) t_vertbuf->bind();
    if (t_elembuf) t_elembuf->bind();

    for (auto& attr : m_attributes) {
        glCheck(glVertexAttribPointer(attr.index, attr.size, attr.cmp_type, (u8)attr.normalized, attr.stride, (const void*)attr.offset));
        glCheck(glEnableVertexAttribArray(attr.index));
    }
}

vertex_array_base::vertex_array_base(vertex_array_base&& t_vertbuf) noexcept {
    *this = std::move(t_vertbuf);
}

vertex_array_base& vertex_array_base::operator=(vertex_array_base&& t_vertbuf) noexcept {
    std::swap(m_id, t_vertbuf.m_id);
    std::swap(m_attributes, t_vertbuf.m_attributes);
    std::swap(m_vertbuf, t_vertbuf.m_vertbuf);
    std::swap(m_elembuf, t_vertbuf.m_elembuf);
    return *this;
}

vertex_array_base::~vertex_array_base() {
    if (m_id) {
        glCheck(glDeleteVertexArrays(1, &m_id));
    }
}

void vertex_array_base::bind() const {
    glCheck(glBindVertexArray(m_id));
}

}
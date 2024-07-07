#include "ff/gfx/vertex_array.hpp"

#include "../external/glew.hpp"

namespace ff {

vertex_array::vertex_array() {
    glCheck(glGenVertexArrays(1, &m_id));
}

vertex_array::vertex_array(vertex_array&& t_vertbuf) noexcept {
    *this = std::move(t_vertbuf);
}

vertex_array& vertex_array::operator=(vertex_array&& t_vertbuf) noexcept {
    std::swap(m_id, t_vertbuf.m_id);
    std::swap(m_attributes, t_vertbuf.m_attributes);
    return *this;
}

vertex_array::~vertex_array() {
    if (m_id) {
        glCheck(glDeleteVertexArrays(1, &m_id));
    }
}

void vertex_array::bind() const {
    glCheck(glBindVertexArray(m_id));
}

void vertex_array::impl_assign_vertex_buffer(u32 start_loc, u32 buf_id, size_t buf_offset, u32 buf_size, std::span<const vertex_attribute> vattrs, u32 divisor) {
    bind();
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, buf_id));

    for (auto& attr : vattrs) {
        auto loc = start_loc + attr.index;
        glCheck(glVertexAttribPointer(loc,
                                      attr.size,
                                      attr.cmp_type,
                                      (u8)attr.normalized,
                                      attr.stride,
                                      (const void*)(buf_offset + attr.offset)));

        glCheck(glEnableVertexAttribArray(loc));
        glCheck(glVertexAttribDivisor(loc, divisor));

        auto it = std::lower_bound(m_attributes.begin(), m_attributes.end(), loc, [](const loc_attr& t_attr, u32 t_loc) {
            return t_attr.loc < t_loc;
        });

        auto n_attr = loc_attr{ loc, buf_size, divisor, true, &attr };
        if (it == m_attributes.end()) {
            m_attributes.push_back(n_attr);
        }
        else if (it->loc == loc) {
            *it = n_attr;
        }
        else {
            m_attributes.insert(it, n_attr);
        }
    }
}

void vertex_array::impl_assign_element_buffer(u32 buf_id, size_t buf_offset, u32 buf_size, u32 index_type) {
    bind();
    glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf_id));
    if (buf_id > 0) {
        m_elements_info = elements_info{
            .index_type    = index_type,
            .count = buf_size,
            .offset        = (const void*)buf_offset
        };
    }
    else {
        m_elements_info.reset();
    }
}










}
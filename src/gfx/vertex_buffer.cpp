#include "ff/gfx/vertex_buffer.hpp"

#include "../external/glew.hpp"

#include <stdexcept>

namespace ff {

vertex_buffer::vertex_buffer()
: m_usage{ buffer_usage::Static }
{
    glCheck(glGenBuffers(1, &m_id));
}

vertex_buffer::vertex_buffer(buffer_usage t_usage)
: m_usage{ buffer_usage::Static }
{
    glCheck(glGenBuffers(1, &m_id));
}

vertex_buffer::vertex_buffer(vertex_buffer&& t_vertbuf) noexcept {
    *this = std::move(t_vertbuf);
}

vertex_buffer& vertex_buffer::operator=(vertex_buffer && t_vertbuf) noexcept {
    std::swap(m_id, t_vertbuf.m_id);
    std::swap(m_bufsize, t_vertbuf.m_bufsize);
    std::swap(m_usage, t_vertbuf.m_usage);
    return *this;
}

vertex_buffer::~vertex_buffer() {
    if (m_id) {
        glCheck(glDeleteBuffers(1, &m_id));
    }
}

void vertex_buffer::bind() const {
    if (m_id) {
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_id));
    }
}

bool vertex_buffer::copy_impl(const void* t_data, size_t t_size, size_t t_offset, bool reset) {
    bind();
    if (!t_data || m_bufsize == 0 || reset) {
        glCheck(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)t_size, t_data, static_cast<GLenum>(m_usage)));
        m_bufsize = t_size;
    }
    else {
        auto end = t_offset + t_size;
        if (end > m_bufsize) {
            return false;
        }
        glCheck(glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)t_offset, (GLsizeiptr)t_size, t_data));
    }
    return true;
}

}
#include "ff/gfx/gpu_buffer.hpp"

#include "../external/glew.hpp"

namespace ff::detail {

constexpr u32 target = GL_COPY_WRITE_BUFFER;

u32 gpu_interface::gen_buffer() {
    u32 id;
    glCheck(glGenBuffers(1, &id));
    return id;
};

void gpu_interface::delete_buffer(u32 t_id) {
    if (t_id != 0) {
        glCheck(glDeleteBuffers(1, &t_id));
    }
};

size_t gpu_interface::copy_buffer(u32 t_id, std::span<const std::byte> data, i64 t_offset_bytes) {
    glCheck(glBindBuffer(target, t_id));
    glCheck(glBufferSubData(target, (i64)t_offset_bytes, data.size_bytes(), data.data()));
    return data.size_bytes();
};

void gpu_interface::reset_buffer(u32 t_id, i64 t_size_bytes, gpu_usage t_usage) {
    glCheck(glBindBuffer(target, t_id));
    glCheck(glBufferData(target, t_size_bytes, nullptr, static_cast<u32>(t_usage)));
};

void gpu_interface::reset_buffer(u32 t_id, std::span<const std::byte> data, gpu_usage t_usage) {
    glCheck(glBindBuffer(target, t_id));
    glCheck(glBufferData(target, data.size_bytes(), data.data(), static_cast<u32>(t_usage)));
};

void* gpu_interface::map_buffer(u32 t_id, gpu_mem_access t_access) {

    u32 access_bit;
    switch (t_access) {
        case gpu_mem_access::R:  access_bit = GL_READ_ONLY;  break;
        case gpu_mem_access::W:  access_bit = GL_WRITE_ONLY; break;
        case gpu_mem_access::RW: access_bit = GL_READ_WRITE; break;
        default: return nullptr;
    }

    glCheck(glBindBuffer(target, t_id));

    void* ptr;
    glCheck(ptr = glMapBuffer(target, access_bit));
    return ptr;
};

void gpu_interface::unmap_buffer(u32 t_id) {
    glCheck(glUnmapBuffer(target));
};

}
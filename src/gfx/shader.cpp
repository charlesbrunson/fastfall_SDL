#include "ff/gfx/shader.hpp"

#include "../external/glew.hpp"
#include "ff/util/log.hpp"

#include <magic_enum.hpp>

namespace ff {

shader::shader()
    : m_id{ 0 }
    , m_uniforms{}
    , m_attributes{}
{
}

shader::shader(u32 t_id, std::vector<shader_uniform>&& t_uniforms, std::vector<shader_attribute>&& t_attributes)
: m_id{ t_id }
, m_uniforms{ std::move(t_uniforms) }
, m_attributes{ std::move(t_attributes) }
{
}

shader::shader(shader&& t_shader) noexcept {
    *this = std::move(t_shader);
}

shader& shader::operator=(shader&& t_shader) noexcept {
    std::swap(m_id, t_shader.m_id);
    std::swap(m_uniforms, t_shader.m_uniforms);
    std::swap(m_attributes, t_shader.m_attributes);
    return *this;
}

shader::~shader() {
    if (m_id) {
        glDeleteShader(m_id);
    }
}

i32 shader::get_loc(std::string_view t_param) const {
    for (auto uni : m_uniforms) {
        if (t_param == uni.name) {
            return uni.id;
        }
    }
    return -1;
}

void shader::set(std::string_view t_param, void* t_valueptr, int t_type, u8vec2 t_extents, bool transpose, i32 t_size) const {
    glUseProgram(m_id);

    i32 loc = get_loc(t_param);
    if (loc < 0) {
        ff::warn("no location for shader param: {}", t_param);
        return;
    }

    struct code {
        int type;
        u8vec2 extents;

        constexpr operator u64() const {
            u64 v = ((u64)type << 32 | (u64)extents.x << 16 | (u64)extents.y);
            return v;
        };
    };

    auto tp = transpose ? GL_TRUE : GL_FALSE;

    switch (code{ t_type, t_extents }) {
        // float
        case code{ type_value_v<f32>, { 1, 1 } }:
            glUniform1fv(loc, t_size, (float*)t_valueptr);
            break;
        case code{ type_value_v<f32>, { 2, 1 } }:
            glUniform2fv(loc, t_size, (float*)t_valueptr);
            break;
        case code{ type_value_v<f32>, { 3, 1 } }:
            glUniform3fv(loc, t_size, (float*)t_valueptr);
            break;
        case code{ type_value_v<f32>, { 4, 1 } }:
            glUniform4fv(loc, t_size, (float*)t_valueptr);
            break;
        case code{ type_value_v<f32>, { 2, 2 } }:
            glUniformMatrix2fv(loc, t_size, tp, (float*)t_valueptr);
            break;
        case code{ type_value_v<f32>, { 3, 3 } }:
            glUniformMatrix3fv(loc, t_size, tp, (float*)t_valueptr);
            break;
        case code{ type_value_v<f32>, { 4, 4 } }:
            glUniformMatrix4fv(loc, t_size, tp, (float*)t_valueptr);
            break;
        case code{ type_value_v<f32>, { 2, 3 } }:
            glUniformMatrix2x3fv(loc, t_size, tp, (float*)t_valueptr);
            break;
        case code{ type_value_v<f32>, { 3, 2 } }:
            glUniformMatrix3x2fv(loc, t_size, tp, (float*)t_valueptr);
            break;
        case code{ type_value_v<f32>, { 2, 4 } }:
            glUniformMatrix2x4fv(loc, t_size, tp, (float*)t_valueptr);
            break;
        case code{ type_value_v<f32>, { 4, 2 } }:
            glUniformMatrix4x2fv(loc, t_size, tp, (float*)t_valueptr);
            break;
        case code{ type_value_v<f32>, { 3, 4 } }:
            glUniformMatrix4x2fv(loc, t_size, tp, (float*)t_valueptr);
            break;
        case code{ type_value_v<f32>, { 4, 3 } }:
            glUniformMatrix4x2fv(loc, t_size, tp, (float*)t_valueptr);
            break;

        // int
        case code{ type_value_v<i32>, { 1, 1 } }:
            glUniform1iv(loc, t_size, (i32*)t_valueptr);
            break;
        case code{ type_value_v<i32>, { 2, 1 } }:
            glUniform2iv(loc, t_size, (i32*)t_valueptr);
            break;
        case code{ type_value_v<i32>, { 3, 1 } }:
            glUniform3iv(loc, t_size, (i32*)t_valueptr);
            break;
        case code{ type_value_v<i32>, { 4, 1 } }:
            glUniform4iv(loc, t_size, (i32*)t_valueptr);

        // uint
        case code{ type_value_v<u32>, { 1, 1 } }:
            glUniform1uiv(loc, t_size, (u32*)t_valueptr);
            break;
        case code{ type_value_v<u32>, { 2, 1 } }:
            glUniform2uiv(loc, t_size, (u32*)t_valueptr);
            break;
        case code{ type_value_v<u32>, { 3, 1 } }:
            glUniform3uiv(loc, t_size, (u32*)t_valueptr);
            break;
        case code{ type_value_v<u32>, { 4, 1 } }:
            glUniform4uiv(loc, t_size, (u32*)t_valueptr);
            break;
        default:
            ff::warn("no uniform set function for type {}, with extents {}",
                     typeid(t_type).name(),
                     glm::to_string(t_extents));
    }
}

shader_factory& shader_factory::add_vertex(std::string_view t_name, std::string_view t_src) {
    return add_shader(t_name, t_src, GL_VERTEX_SHADER, m_vertex_src);
}

shader_factory& shader_factory::add_fragment(std::string_view t_name, std::string_view t_src) {
    return add_shader(t_name, t_src, GL_FRAGMENT_SHADER, m_fragment_src);
}

shader_factory& shader_factory::add_shader(std::string_view t_name, std::string_view t_src, int shader_type, std::optional<compiled_shader>& dest) {
    if (dest && dest->id) {
        glDeleteShader(dest->id);
        dest->id = 0;
    }

    compiled_shader shader_info{
        .compiled = false,
        .id       = glCreateShader(shader_type),
        .name     = std::string{ t_name },
        .source   = std::string{ t_src },
        .error    = {}
    };

    char* src = shader_info.source.data();
    i32 len   = static_cast<i32>(shader_info.source.length());
    glShaderSource(shader_info.id, 1, &src, &len);
    glCompileShader(shader_info.id);

    i32 result;
    glGetShaderiv(shader_info.id, GL_COMPILE_STATUS, &result);
    if (!result) {
        constexpr u32 log_len = 1024;
        char log_info[log_len];
        i32 out_len;
        glGetShaderInfoLog(shader_info.id, log_len, &out_len, &log_info[0]);

        shader_info.error = { &log_info[0], static_cast<u32>(out_len) };
        ff::error("{} shader compilation: {}", t_name, shader_info.error);

        glDeleteShader(shader_info.id);
        shader_info.id = 0;
    }
    else {
        shader_info.compiled = true;
    }
    dest = shader_info;
    return *this;
}

std::optional<shader> shader_factory::build() {
    if (m_vertex_src && m_fragment_src) {

        auto program_id = glCreateProgram();
        glAttachShader(program_id, m_vertex_src->id);
        glAttachShader(program_id, m_fragment_src->id);
        glLinkProgram(program_id);

        i32 result;
        bool linked = true;
        glGetProgramiv(program_id, GL_LINK_STATUS, &result);
        if (!result) {
            constexpr u32 log_len = 1024;
            char log_info[log_len];
            i32 out_len;
            glGetProgramInfoLog(program_id, log_len, &out_len, &log_info[0]);

            std::string_view log_msg{ &log_info[0], static_cast<u32>(out_len) };
            ff::error("shader linker: {}", log_msg);

            glDeleteProgram(program_id);
            linked = false;
        }

        glDeleteShader(m_vertex_src->id);
        glDeleteShader(m_fragment_src->id);

        *this = {};

        if (linked) {
            int count;
            constexpr int bufsize = 32;
            int buflen = 0;

            shader_uniform uni_info;
            glGetProgramiv(program_id, GL_ACTIVE_UNIFORMS, &count);
            std::vector<shader_uniform> uniforms(count);
            for (uni_info.id = 0; uni_info.id < count; uni_info.id++) {
                memset(uni_info.name, 0, sizeof(uni_info.name));
                glGetActiveUniform(program_id, (GLuint)uni_info.id, bufsize, &buflen, &uni_info.size, &uni_info.type, uni_info.name);
                uniforms[uni_info.id] = uni_info;
                ff::info("uniform   {} - {}: {}[{}]", uni_info.id, uni_info.name, uni_info.type, uni_info.size);
            }

            shader_attribute attr_info;
            glGetProgramiv(program_id, GL_ACTIVE_ATTRIBUTES, &count);
            std::vector<shader_attribute> attributes(count);
            for (attr_info.id = 0; attr_info.id < count; attr_info.id++) {
                memset(attr_info.name, 0, sizeof(attr_info.name));
                glGetActiveAttrib(program_id, (GLuint)attr_info.id, bufsize, &buflen, &attr_info.size, &attr_info.type, attr_info.name);
                attributes[attr_info.id] = attr_info;
                ff::info("attribute {} - {}: {}[{}]", attr_info.id, attr_info.name, attr_info.type, attr_info.size);
            }

            return shader{ program_id, std::move(uniforms), std::move(attributes) };
        }
    }
    return {};
}

}

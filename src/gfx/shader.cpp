#include "ff/gfx/shader.hpp"

#include "../external/glew.hpp"
#include "ff/util/log.hpp"

#include <magic_enum.hpp>

namespace ff {

namespace detail {

struct compiled_shader {
    u32 id = 0;
    std::string source;
    std::string error;
};

std::optional<compiled_shader> add_shader(std::string_view t_src, int shader_type) {
    compiled_shader shader_info{
        .id       = glCreateShader(shader_type),
        .source   = std::string{t_src},
        .error    = {}
    };

    char *src = shader_info.source.data();
    i32 len = static_cast<i32>(shader_info.source.length());
    glCheck(glShaderSource(shader_info.id, 1, &src, &len));
    glCheck(glCompileShader(shader_info.id));

    i32 result;
    glCheck(glGetShaderiv(shader_info.id, GL_COMPILE_STATUS, &result));
    if (!result) {
        constexpr u32 log_len = 1024;
        char log_info[log_len];
        i32 out_len;
        glCheck(glGetShaderInfoLog(shader_info.id, log_len, &out_len, &log_info[0]));

        shader_info.error = {&log_info[0], static_cast<u32>(out_len)};
        ff::error("shader compilation: {}", shader_info.error);

        glCheck(glDeleteShader(shader_info.id));
        return {};
    }
    return shader_info;
}

}

shader::shader()
: m_id{ 0 }
, m_uniforms{}
, m_attributes{}
{
}

shader::shader(std::string_view t_vert_src, std::string_view t_frag_src)
{
    auto vert_shader = detail::add_shader(t_vert_src, GL_VERTEX_SHADER);
    auto frag_shader = detail::add_shader(t_frag_src, GL_FRAGMENT_SHADER);

    if (vert_shader && frag_shader) {
        build(vert_shader->id, frag_shader->id);
    }
}

/*
shader::shader(u32 t_id, std::vector<shader_uniform>&& t_uniforms, std::vector<shader_attribute>&& t_attributes)
: m_id{ t_id }
, m_uniforms{ std::move(t_uniforms) }
, m_attributes{ std::move(t_attributes) }
{
}
*/

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
        glCheck(glDeleteProgram(m_id));
    }
}

void shader::bind() const {
    glCheck(glUseProgram(m_id));
}

i32 shader::get_loc(std::string_view t_param) const {
    for (auto uni : m_uniforms) {
        if (t_param == std::string_view{ uni.name }) {
            return uni.loc;
        }
    }
    return -1;
}

void shader::set(std::string_view t_param, const void* t_valueptr, i32 t_type, u16vec2 t_extents, bool transpose, i32 t_size) const {

    i32 loc = get_loc(t_param);
    if (loc < 0) {
        ff::warn("no location for shader param: {}", t_param);
        return;
    }

    struct code {
        int type;
        u16vec2 extents;

        constexpr operator u64() const {
            u64 v = (((u64)type << 32) | ((u64)extents.x << 16) | ((u64)extents.y));
            return v;
        };
    };

    auto tp = transpose ? GL_TRUE : GL_FALSE;

    switch (code{ t_type, t_extents }) {
        // float
        case code{ type_value_v<f32>, { 1, 1 } }:
            glCheck(glUniform1fv(loc, t_size, (const f32*)t_valueptr));
            break;
        case code{ type_value_v<f32>, { 2, 1 } }:
            glCheck(glUniform2fv(loc, t_size, (const f32*)t_valueptr));
            break;
        case code{ type_value_v<f32>, { 3, 1 } }:
            glCheck(glUniform3fv(loc, t_size, (const f32*)t_valueptr));
            break;
        case code{ type_value_v<f32>, { 4, 1 } }:
            glCheck(glUniform4fv(loc, t_size, (const f32*)t_valueptr));
            break;
        case code{ type_value_v<f32>, { 2, 2 } }:
            glCheck(glUniformMatrix2fv(loc, t_size, tp, (const f32*)t_valueptr));
            break;
        case code{ type_value_v<f32>, { 3, 3 } }:
            glCheck(glUniformMatrix3fv(loc, t_size, tp, (const f32*)t_valueptr));
            break;
        case code{ type_value_v<f32>, { 4, 4 } }:
            glCheck(glUniformMatrix4fv((GLint)loc, (GLsizei)t_size, (GLboolean)tp, (const GLfloat*)t_valueptr));
            break;
        case code{ type_value_v<f32>, { 2, 3 } }:
            glCheck(glUniformMatrix2x3fv(loc, t_size, tp, (const f32*)t_valueptr));
            break;
        case code{ type_value_v<f32>, { 3, 2 } }:
            glCheck(glUniformMatrix3x2fv(loc, t_size, tp, (const f32*)t_valueptr));
            break;
        case code{ type_value_v<f32>, { 2, 4 } }:
            glCheck(glUniformMatrix2x4fv(loc, t_size, tp, (const f32*)t_valueptr));
            break;
        case code{ type_value_v<f32>, { 4, 2 } }:
            glCheck(glUniformMatrix4x2fv(loc, t_size, tp, (const f32*)t_valueptr));
            break;
        case code{ type_value_v<f32>, { 3, 4 } }:
            glCheck(glUniformMatrix3x4fv(loc, t_size, tp, (const f32*)t_valueptr));
            break;
        case code{ type_value_v<f32>, { 4, 3 } }:
            glCheck(glUniformMatrix4x3fv(loc, t_size, tp, (const f32*)t_valueptr));
            break;

        // int
        case code{ type_value_v<i32>, { 1, 1 } }:
            glCheck(glUniform1iv(loc, t_size, (const i32*)t_valueptr));
            break;
        case code{ type_value_v<i32>, { 2, 1 } }:
            glCheck(glUniform2iv(loc, t_size, (const i32*)t_valueptr));
            break;
        case code{ type_value_v<i32>, { 3, 1 } }:
            glCheck(glUniform3iv(loc, t_size, (const i32*)t_valueptr));
            break;
        case code{ type_value_v<i32>, { 4, 1 } }:
            glCheck(glUniform4iv(loc, t_size, (const i32*)t_valueptr));

        // uint
        case code{ type_value_v<u32>, { 1, 1 } }:
            glCheck(glUniform1uiv(loc, t_size, (const u32*)t_valueptr));
            break;
        case code{ type_value_v<u32>, { 2, 1 } }:
            glCheck(glUniform2uiv(loc, t_size, (const u32*)t_valueptr));
            break;
        case code{ type_value_v<u32>, { 3, 1 } }:
            glCheck(glUniform3uiv(loc, t_size, (const u32*)t_valueptr));
            break;
        case code{ type_value_v<u32>, { 4, 1 } }:
            glCheck(glUniform4uiv(loc, t_size, (const u32*)t_valueptr));
            break;
        default:
            ff::warn("no uniform set function for type {}, with extents {}",
                     typeid(t_type).name(),
                     glm::to_string(t_extents));
    }
}

bool shader::build(u32 t_vert_id, u32 t_frag_id) {

    m_id = glCreateProgram();
    glCheck(glAttachShader(m_id, t_vert_id));
    glCheck(glAttachShader(m_id, t_frag_id));
    glCheck(glLinkProgram(m_id));

    i32 result;
    bool linked = true;
    glCheck(glGetProgramiv(m_id, GL_LINK_STATUS, &result));
    if (!result) {
        constexpr u32 log_len = 1024;
        char log_info[log_len];
        i32 out_len;
        glCheck(glGetProgramInfoLog(m_id, log_len, &out_len, &log_info[0]));

        std::string_view log_msg{ &log_info[0], static_cast<u32>(out_len) };
        ff::error("shader linker: {}", log_msg);

        glCheck(glDeleteProgram(m_id));
        m_id = 0;
        linked = false;
    }

    glCheck(glDeleteShader(t_vert_id));
    glCheck(glDeleteShader(t_frag_id));

    if (linked) {
        int count;
        constexpr int bufsize = 32;
        int buflen = 0;

        shader_uniform uni_info;
        glCheck(glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &count));
        m_uniforms = std::vector<shader_uniform>(count);
        for (uni_info.id = 0; uni_info.id < count; uni_info.id++) {
            memset(uni_info.name, 0, sizeof(uni_info.name));
            glCheck(glGetActiveUniform(m_id, (GLuint)uni_info.id, bufsize, &buflen, &uni_info.size, &uni_info.type, uni_info.name));
            uni_info.loc = glGetUniformLocation(m_id, uni_info.name);
            m_uniforms[uni_info.id] = uni_info;
            ff::info("uniform   {} - {}: {}[{}]", uni_info.id, uni_info.name, uni_info.type, uni_info.size);
        }

        shader_attribute attr_info;
        glCheck(glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &count));
        m_attributes = std::vector<shader_attribute>(count);
        for (attr_info.id = 0; attr_info.id < count; attr_info.id++) {
            memset(attr_info.name, 0, sizeof(attr_info.name));
            glCheck(glGetActiveAttrib(m_id, (GLuint)attr_info.id, bufsize, &buflen, &attr_info.size, &attr_info.type, attr_info.name));
            m_attributes[attr_info.id] = attr_info;
            ff::info("attribute {} - {}: {}[{}]", attr_info.id, attr_info.name, attr_info.type, attr_info.size);
        }
        return true;
    }
    return false;
}

/*

shader_factory& shader_factory::add_vertex(std::string_view t_name, std::string_view t_src) {
    return add_shader(t_name, t_src, GL_VERTEX_SHADER, m_vertex_src);
}

shader_factory& shader_factory::add_fragment(std::string_view t_name, std::string_view t_src) {
    return add_shader(t_name, t_src, GL_FRAGMENT_SHADER, m_fragment_src);
}

shader_factory& shader_factory::add_shader(std::string_view t_name, std::string_view t_src, int shader_type, std::optional<compiled_shader>& dest) {
    if (dest && dest->id) {
        glCheck(glDeleteShader(dest->id));
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
    glCheck(glShaderSource(shader_info.id, 1, &src, &len));
    glCheck(glCompileShader(shader_info.id));

    i32 result;
    glCheck(glGetShaderiv(shader_info.id, GL_COMPILE_STATUS, &result));
    if (!result) {
        constexpr u32 log_len = 1024;
        char log_info[log_len];
        i32 out_len;
        glCheck(glGetShaderInfoLog(shader_info.id, log_len, &out_len, &log_info[0]));

        shader_info.error = { &log_info[0], static_cast<u32>(out_len) };
        ff::error("{} shader compilation: {}", t_name, shader_info.error);

        glCheck(glDeleteShader(shader_info.id));
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
        glCheck(glAttachShader(program_id, m_vertex_src->id));
        glCheck(glAttachShader(program_id, m_fragment_src->id));
        glCheck(glLinkProgram(program_id));

        i32 result;
        bool linked = true;
        glCheck(glGetProgramiv(program_id, GL_LINK_STATUS, &result));
        if (!result) {
            constexpr u32 log_len = 1024;
            char log_info[log_len];
            i32 out_len;
            glCheck(glGetProgramInfoLog(program_id, log_len, &out_len, &log_info[0]));

            std::string_view log_msg{ &log_info[0], static_cast<u32>(out_len) };
            ff::error("shader linker: {}", log_msg);

            glCheck(glDeleteProgram(program_id));
            linked = false;
        }

        glCheck(glDeleteShader(m_vertex_src->id));
        glCheck(glDeleteShader(m_fragment_src->id));

        *this = {};

        if (linked) {
            int count;
            constexpr int bufsize = 32;
            int buflen = 0;

            shader_uniform uni_info;
            glCheck(glGetProgramiv(program_id, GL_ACTIVE_UNIFORMS, &count));
            std::vector<shader_uniform> uniforms(count);
            for (uni_info.id = 0; uni_info.id < count; uni_info.id++) {
                memset(uni_info.name, 0, sizeof(uni_info.name));
                glCheck(glGetActiveUniform(program_id, (GLuint)uni_info.id, bufsize, &buflen, &uni_info.size, &uni_info.type, uni_info.name));
                uni_info.loc = glGetUniformLocation(program_id, uni_info.name);
                uniforms[uni_info.id] = uni_info;
                ff::info("uniform   {} - {}: {}[{}]", uni_info.id, uni_info.name, uni_info.type, uni_info.size);
            }

            shader_attribute attr_info;
            glCheck(glGetProgramiv(program_id, GL_ACTIVE_ATTRIBUTES, &count));
            std::vector<shader_attribute> attributes(count);
            for (attr_info.id = 0; attr_info.id < count; attr_info.id++) {
                memset(attr_info.name, 0, sizeof(attr_info.name));
                glCheck(glGetActiveAttrib(program_id, (GLuint)attr_info.id, bufsize, &buflen, &attr_info.size, &attr_info.type, attr_info.name));
                attributes[attr_info.id] = attr_info;
                ff::info("attribute {} - {}: {}[{}]", attr_info.id, attr_info.name, attr_info.type, attr_info.size);
            }

            return shader{ program_id, std::move(uniforms), std::move(attributes) };
        }
    }
    return {};
}

*/

}

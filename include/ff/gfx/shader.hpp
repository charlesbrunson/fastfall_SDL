#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <optional>

#include "ff/util/math.hpp"

namespace ff {
enum class uniform_type : u8 {
    Float,
    Int,
    Uint,
};

namespace detail {
template<class T>
concept uniform_datatype = (std::same_as<T, f32> || std::same_as<T, i32> || std::same_as<T, u32>);

template<class T>
struct to_uniform_type {};

template<>
struct to_uniform_type<f32> {
    constexpr static uniform_type type = uniform_type::Float;
};

template<>
struct to_uniform_type<i32> {
    constexpr static uniform_type type = uniform_type::Int;
};

template<>
struct to_uniform_type<u32> {
    constexpr static uniform_type type = uniform_type::Uint;
};
}

struct shader_uniform {
    char name[32] = {};
    i32 id   = -1;
    u32 type = 0;
    i32 size = 0;
};
struct shader_attribute {
    char name[32] = {};
    i32 id   = -1;
    u32 type = 0;
    i32 size = 0;
};

class shader_factory;

class shader {
    friend class shader_factory;
    shader(u32 t_id, std::vector<shader_uniform>&& t_uniforms, std::vector<shader_attribute>&& t_attributes);
public:
    shader();
    shader(const shader& t_shader) = delete;
    shader& operator=(const shader& t_shader) = delete;
    shader(shader&& t_shader) noexcept;
    shader& operator=(shader&& t_shader) noexcept;
    ~shader();

    inline u32 id() const { return m_id; }
    i32 get_loc(std::string_view t_param) const;

    template<detail::uniform_datatype T>
    inline void set(std::string_view t_param, const T& t_value) {
        set(t_param, &t_value, detail::to_uniform_type<T>::type, { 1, 1 }, false, 1);
    }

    template<detail::uniform_datatype T>
    inline void set(std::string_view t_param, const std::span<T> t_value) {
        set(t_param, &t_value[0], detail::to_uniform_type<T>::type, { 1, 1 }, false, t_value.size());
    }

    template<detail::uniform_datatype T, u8 Extent1>
    requires (Extent1 > 0 && Extent1 <= 4)
    inline void set(std::string_view t_param, const vec<Extent1, T>& t_value) {
        set(t_param, &t_value, detail::to_uniform_type<T>::type, { Extent1, 1 }, false, 1);
    }

    template<detail::uniform_datatype T, u8 Extent1>
    requires (Extent1 > 0 && Extent1 <= 4)
    inline void set(std::string_view t_param, const std::span<vec<Extent1, T>> t_value) {
        set(t_param, &t_value[0], detail::to_uniform_type<T>::type, { Extent1, 1 }, false, t_value.size());
    }

    template<detail::uniform_datatype T, u8 Extent1, u8 Extent2>
    requires (Extent1 > 1 && Extent1 <= 4 && Extent2 > 1 && Extent2 <= 4)
    inline void set(std::string_view t_param, const mat<Extent1, Extent2, T>& t_value) {
        set(t_param, &t_value, detail::to_uniform_type<T>::type, { Extent1, Extent2 }, false, 1);
    }

    template<detail::uniform_datatype T, u8 Extent1, u8 Extent2>
    requires (Extent1 > 1 && Extent1 <= 4 && Extent2 > 1 && Extent2 <= 4)
    inline void set(std::string_view t_param, const std::span<mat<Extent1, Extent2, T>> t_value) {
        set(t_param, &t_value[0], detail::to_uniform_type<T>::type, { Extent1, Extent2 }, false, t_value.size());
    }

private:
    void set(std::string_view t_param, void* t_valueptr, uniform_type t_type, u8vec2 t_extents, bool transpose, i32 t_size) const;


    std::vector<shader_uniform> m_uniforms;
    std::vector<shader_attribute> m_attributes;
    uint32_t m_id = 0;
};

class shader_factory {
    struct compiled_shader {
        bool compiled = false;
        u32 id = 0;
        std::string name;
        std::string source;
        std::string error;
    };
public:
    shader_factory& add_vertex(std::string_view t_name, std::string_view t_src);
    shader_factory& add_fragment(std::string_view t_name, std::string_view t_src);
    std::optional<shader> build();

private:
    shader_factory& add_shader(std::string_view t_name, std::string_view t_src, int shader_type, std::optional<compiled_shader>& dest);

    std::optional<compiled_shader> m_vertex_src;
    std::optional<compiled_shader> m_fragment_src;
};

}
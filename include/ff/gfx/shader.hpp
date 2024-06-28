#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <optional>

#include "ff/util/math.hpp"

namespace ff {
enum class uniform_type {
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

struct uniform {
    char parameter_name[64];
    u32 location;
    uniform_type type;
    u8 extent1;
    u8 extent2;
    size_t count;
};

class shader_factory;

class shader {
    friend class shader_factory;
    shader(u32 t_id);
public:
    shader(const shader& t_shader) = delete;
    shader& operator=(const shader& t_shader) = delete;
    shader(shader&& t_shader) noexcept;
    shader& operator=(shader&& t_shader) noexcept;
    ~shader();

    inline uint32_t id() const { return m_id; }
    u32 get_loc(std::string_view t_param) const;

    template<detail::uniform_datatype T>
    inline void set(std::string_view t_param, const T& t_value) {
        set(t_param, &t_value, detail::to_uniform_type<T>::type, 1, 1);
    }

    template<detail::uniform_datatype T>
    inline void set(std::string_view t_param, const std::span<T> t_value) {
        set(t_param, &t_value[0], detail::to_uniform_type<T>::type, 1, 1, t_value.size());
    }

    template<detail::uniform_datatype T, u8 Extent1>
    requires (Extent1 > 0 && Extent1 <= 4)
    inline void set(std::string_view t_param, const vec<Extent1, T>& t_value) {
        set(t_param, &t_value, detail::to_uniform_type<T>::type, { Extent1, 1 });
    }

    template<detail::uniform_datatype T, u8 Extent1>
    requires (Extent1 > 0 && Extent1 <= 4)
    inline void set(std::string_view t_param, const std::span<vec<Extent1, T>> t_value) {
        set(t_param, &t_value[0], detail::to_uniform_type<T>::type, { Extent1, 1 }, t_value.size());
    }

    template<detail::uniform_datatype T, u8 Extent1, u8 Extent2>
    requires (Extent1 > 1 && Extent1 <= 4 && Extent2 > 1 && Extent2 <= 4)
    inline void set(std::string_view t_param, const mat<Extent1, Extent2, T>& t_value) {
        set(t_param, &t_value, detail::to_uniform_type<T>::type, { Extent1, Extent2 });
    }

    template<detail::uniform_datatype T, u8 Extent1, u8 Extent2>
    requires (Extent1 > 1 && Extent1 <= 4 && Extent2 > 1 && Extent2 <= 4)
    inline void set(std::string_view t_param, const std::span<mat<Extent1, Extent2, T>> t_value) {
        set(t_param, &t_value[0], detail::to_uniform_type<T>::type, { Extent1, Extent2 }, t_value.size());
    }

private:
    void set(std::string_view t_param, void* t_valueptr, uniform_type t_type, u8vec2 t_extents, size_t t_size = 1) const;

    std::vector<uniform> m_uniforms;
    uint32_t m_id;
};

class shader_factory {
    struct compiled_shader {
        bool compiled = false;
        u32 id = 0;
        std::string source;
        std::string error;
    };
public:
    shader_factory& add_vertex(std::string_view t_src);
    shader_factory& add_fragment(std::string_view t_src);
    std::optional<shader> build() const;
    std::string error() const;
private:
    std::optional<compiled_shader> m_vertex_src;
    std::optional<compiled_shader> m_fragment_src;
};

}
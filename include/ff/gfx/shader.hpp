#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <optional>

#include "ff/util/math.hpp"

namespace ff {

namespace detail {
template<class T>
concept uniform_datatype = (std::same_as<T, f32> || std::same_as<T, i32> || std::same_as<T, u32>);
}

struct shader_uniform {
    char name[32] = {};
    i32 id   = -1;
    i32 loc  = -1;
    u32 type = 0;
    i32 size = 0;
};
struct shader_attribute {
    char name[32] = {};
    i32 id   = -1;
    u32 type = 0;
    i32 size = 0;
};

class shader {
public:
    shader();
    shader(std::string_view t_vert_src, std::string_view t_frag_src);
    shader(const shader& t_shader) = delete;
    shader& operator=(const shader& t_shader) = delete;
    shader(shader&& t_shader) noexcept;
    shader& operator=(shader&& t_shader) noexcept;
    ~shader();

    void bind() const;
    inline u32 id() const { return m_id; }
    i32 get_loc(std::string_view t_param) const;

    [[nodiscard]] inline bool valid() const { return m_id != 0; }
    [[nodiscard]] inline explicit operator bool() const { return m_id != 0; }

    template<detail::uniform_datatype T>
    inline void set(std::string_view t_param, const T& t_value) {
        set(t_param, (const void*)&t_value, type_value_v<T>, { 1, 1 }, false, 1);
    }

    template<detail::uniform_datatype T>
    inline void set(std::string_view t_param, std::span<T> t_value) {
        set(t_param, (const void*)&t_value[0], type_value_v<T>, { 1, 1 }, false, t_value.size());
    }

    template<detail::uniform_datatype T, i32 Extent1>
    requires (Extent1 > 0 && Extent1 <= 4)
    inline void set(std::string_view t_param, const vec<Extent1, T>& t_value) {
        set(t_param, (const void*)value_ptr(t_value), type_value_v<T>, { Extent1, 1 }, false, 1);
    }

    template<detail::uniform_datatype T, i32 Extent1>
    requires (Extent1 > 0 && Extent1 <= 4)
    inline void set(std::string_view t_param, std::span<vec<Extent1, T>> t_value) {
        set(t_param, (const void*)value_ptr(t_value[0]), type_value_v<T>, { Extent1, 1 }, false, t_value.size());
    }

    template<detail::uniform_datatype T, i32 Extent1, i32 Extent2>
    requires (Extent1 > 1 && Extent1 <= 4 && Extent2 > 1 && Extent2 <= 4)
    inline void set(std::string_view t_param, const mat<Extent1, Extent2, T>& t_value) {
        set(t_param, (const void*)value_ptr(t_value), type_value_v<T>, { Extent1, Extent2 }, false, 1);
    }

    template<detail::uniform_datatype T, i32 Extent1, i32 Extent2>
    requires (Extent1 > 1 && Extent1 <= 4 && Extent2 > 1 && Extent2 <= 4)
    inline void set(std::string_view t_param, std::span<mat<Extent1, Extent2, T>> t_value) {
        set(t_param, (const void*)value_ptr(t_value[0]), type_value_v<T>, { Extent1, Extent2 }, false, t_value.size());
    }

private:
    void set(std::string_view t_param, const void* t_valueptr, i32 t_type, u16vec2 t_extents, bool transpose, i32 t_size) const;

    bool build(u32 t_vert_id, u32 t_frag_id);

    std::vector<shader_uniform> m_uniforms;
    std::vector<shader_attribute> m_attributes;
    uint32_t m_id = 0;
};

}
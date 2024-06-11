#pragma once

#include "ff/gfx/vertex.hpp"
#include <GL/gl.h>

namespace ff {

template<class Vertex, class Func>
requires std::invocable<Func, GLuint, GLint, GLenum, GLboolean, GLsizei, const void*>
void for_vertex_attr(Func&& func) {
    using attr_list_type = typename Vertex::attributes;
    [&]<class... Ts>(v_attr_list<Ts...>) {
        size_t index = 0;
        size_t offset = 0;
        ([&]<size_t S, class T, bool N>(v_attr<S, T, N>) {

            GLint type = get_type_enum<T>();
            func(index, S, type, N, sizeof(Vertex), (const void*)offset);

            ++index;
            offset += sizeof(T) * S;
        }(Ts{}), ...);
    }(attr_list_type{});
}

template<class T>
constexpr GLint get_type_enum() {
    if      constexpr (std::same_as<T, uint8_t>) {
        return GL_UNSIGNED_BYTE;
    }
    else if constexpr (std::same_as<T, uint16_t>) {
        return GL_UNSIGNED_SHORT;
    }
    else if constexpr (std::same_as<T, uint32_t>) {
        return GL_UNSIGNED_INT;
    }
    else if constexpr (std::same_as<T, int8_t>) {
        return GL_BYTE;
    }
    else if constexpr (std::same_as<T, int16_t>) {
        return GL_SHORT;
    }
    else if constexpr (std::same_as<T, int32_t>) {
        return GL_INT;
    }
    else if constexpr (std::same_as<T, float>) {
        return GL_UNSIGNED_INT;
    }
    else if constexpr (std::same_as<T, double>) {
        return GL_DOUBLE;
    }
    else {
        [](){ static_assert(false, "invalid type"); };
    }
}

}
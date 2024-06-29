#pragma once

#include "color.hpp"

#include <tuple>
#include <array>

namespace ff {

template<size_t S>
concept cmp_size = S > 0 && S <= 4;

template<size_t S, class T, bool N = false>
requires cmp_size<S>
struct v_attr : public std::false_type {};

template<size_t S, bool N> struct v_attr<S, uint8_t,  N>     : public std::true_type {};
template<size_t S, bool N> struct v_attr<S, uint16_t, N>     : public std::true_type {};
template<size_t S, bool N> struct v_attr<S, uint32_t, N>     : public std::true_type {};
template<size_t S, bool N> struct v_attr<S, int8_t,   N>     : public std::true_type {};
template<size_t S, bool N> struct v_attr<S, int16_t,  N>     : public std::true_type {};
template<size_t S, bool N> struct v_attr<S, int32_t,  N>     : public std::true_type {};
template<size_t S>         struct v_attr<S, float,    false> : public std::true_type {};
template<size_t S>         struct v_attr<S, double,   false> : public std::true_type {};

template<class... Ts>
requires (Ts::value && ...)
constexpr size_t v_calc_attr_size() {
    return ([]<size_t S, class T, bool N>(v_attr<S, T, N>) -> size_t {
        return sizeof(T) * S;
    }(Ts{}) + ... + 0);
};

template<class... Ts>
requires (Ts::value && ...)
struct v_attr_list {
    constexpr static size_t size = sizeof...(Ts);
    constexpr static size_t memsize = v_calc_attr_size<Ts...>();
};

template<class Vertex, class... Ts>
concept is_vertex = requires (Vertex a) {
    std::same_as<typename Vertex::attributes, v_attr_list<Ts...>>;
} && sizeof(Vertex) == Vertex::attributes::memsize;

struct attribute_info {

    attribute_info() = default;

    template<size_t S, class T, bool N = false>
    attribute_info(uint32_t ndx, v_attr<S, T, N>, uint32_t strd, const void* off)
        : index{ndx}
        , size{static_cast<int32_t>(S)}
        , cmp_type{type_value_v<T>}
        , normalized{N}
        , stride{strd}
        , offset{off}
    {
    }

    u32  index      = 0;
    i32  size       = 0;
    i32  cmp_type   = 0;
    bool normalized = false;
    u32  stride     = 0;
    const void* offset     = (void*)0;
};

template<is_vertex V>
std::array<attribute_info, V::attributes::size> get_vertex_attribute_info() {
    std::array<attribute_info, V::attributes::size> array;

    using attr_list_type = typename V::attributes;
    [&]<class... Ts>(v_attr_list<Ts...>) {
        uint32_t index = 0;
        size_t offset = 0;
        ([&]<size_t S, class T, bool N>(v_attr<S, T, N>) {

            array[index] = attribute_info{
                index, v_attr<S, T, N>{}, static_cast<uint32_t>(sizeof(V)), (const void*)offset
            };

            ++index;
            offset += sizeof(T) * S;
        }(Ts{}), ...);

    }(attr_list_type{});

    return array;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

struct vertex {
    vec3 pos;
    u16vec2 tex_pos;
    color col;

    using attributes = v_attr_list<
        v_attr<3, float>,
        v_attr<2, u16,     true>,
        v_attr<4, uint8_t, true>
    >;
};
static_assert(is_vertex<vertex>);
static_assert(vertex::attributes::size == 3);
static_assert(vertex::attributes::memsize == 20);

}
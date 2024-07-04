#pragma once

#include "ff/util/math.hpp"
#include "ff/gfx/color.hpp"

#include <type_traits>
#include <array>
#include <tuple>

#include <boost/pfr/core.hpp>

namespace ff {

template<size_t S>
concept cmp_size = S > 0 && S <= 4;

template<class T> struct is_vertex_attr : public std::false_type {};

template<> struct is_vertex_attr<i8>  : public std::true_type {};
template<> struct is_vertex_attr<i16> : public std::true_type {};
template<> struct is_vertex_attr<i32> : public std::true_type {};

template<> struct is_vertex_attr<u8>  : public std::true_type {};
template<> struct is_vertex_attr<u16> : public std::true_type {};
template<> struct is_vertex_attr<u32> : public std::true_type {};

template<> struct is_vertex_attr<f32> : public std::true_type {};
template<> struct is_vertex_attr<f64> : public std::true_type {};

template<> struct is_vertex_attr<color> : public std::true_type {};

template<class T>
inline constexpr bool is_vertex_attr_v = is_vertex_attr<T>::value;

template<class T, size_t E1>
requires (is_vertex_attr_v<T> && cmp_size<E1>)
struct is_vertex_attr<vec<E1, T>> : public std::true_type {};

template<class T, size_t E1, size_t E2>
requires (is_vertex_attr_v<T> && cmp_size<E1> && cmp_size<E2>)
struct is_vertex_attr<mat<E1, E2, T>> : public std::true_type {};

template<class T>
requires is_vertex_attr_v<T>
struct vertex_attr_traits {
    constexpr static uvec2 extents = { 1, 1 };
    constexpr static size_t size = extents.x * extents.y;
    using component_type = T;
};

template<class T, size_t E1>
requires is_vertex_attr_v<T>
struct vertex_attr_traits<vec<E1, T>> {
    constexpr static uvec2 extents = { E1, 1 };
    constexpr static size_t size = extents.x * extents.y;
    using component_type = T;
};

template<>
struct vertex_attr_traits<color> : public vertex_attr_traits<u8vec4> {};

template<class T, size_t E1, size_t E2>
requires is_vertex_attr_v<T>
struct vertex_attr_traits<mat<E1, E2, T>> {
    constexpr static uvec2 extents = { E1, E2 };
    constexpr static size_t size = extents.x * extents.y;
    using component_type = T;
};

template<class T>
constexpr bool validate_vertex_struct() {
    using vertex_tuple = decltype(boost::pfr::structure_to_tuple(std::declval<T>()));
    constexpr size_t size = std::tuple_size_v<vertex_tuple>;

    return []<size_t... I>(std::index_sequence<I...> seq) {
        return (is_vertex_attr_v<std::tuple_element_t<I, vertex_tuple>> && ...);
    }(std::make_index_sequence<size>{});
};

template<class T> struct is_vertex_struct : public std::false_type {};

template<class T>
requires (std::is_standard_layout_v<T> && std::is_aggregate_v<T> && validate_vertex_struct<T>())
struct is_vertex_struct<T> : public std::true_type {};

template<class T>
inline constexpr bool is_vertex_struct_v = is_vertex_struct<T>::value;

struct vertex_attribute {
    u32  index      = 0;
    i32  size       = 0;
    i32  cmp_type   = 0;
    bool normalized = false;
    u32  stride     = 0;
    size_t offset   = 0;
};


template<class T>
concept has_normalized_attributes = requires(T x) {
    { T::normalized::size() > 0 };
};


template<class T>
requires is_vertex_struct_v<T>
struct vertex_traits {
    using tuple_t = decltype(boost::pfr::structure_to_tuple(std::declval<T>()));
    constexpr static size_t size = std::tuple_size_v<tuple_t>;

    constexpr static std::array<vertex_attribute, size> attributes = []() {
        std::array<vertex_attribute, size> array;

        auto is_normalized = [](size_t N) -> bool {
            if constexpr ( has_normalized_attributes<T> ) {
                return [N]<size_t... I>(std::index_sequence<I...> seq) {
                    return ((N == I) || ...);
                }( typename T::normalized{} );
            }
            else {
                return false;
            }
        };

        [&]<size_t... I>(std::index_sequence<I...> seq) {

            uint32_t index = 0;
            size_t offset = 0;
            ([&]() {
                size_t size = vertex_attr_traits<std::tuple_element_t<I, tuple_t>>::size;
                i32 cmp_type = type_value_v<typename vertex_attr_traits<std::tuple_element_t<I, tuple_t>>::component_type>;

                array[index] = vertex_attribute{
                    .index      = index,
                    .size       = (i32)size,
                    .cmp_type   = cmp_type,
                    .normalized = is_normalized(I),
                    .stride     = sizeof(T),
                    .offset     = offset,
                };

                ++index;
                offset += sizeof(std::tuple_element_t<I, tuple_t>);
            }(), ...);

        }(std::make_index_sequence<size>{});

        return array;
    }();
};

template<class T>
concept is_vertex = is_vertex_struct_v<T>;

}
#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>


namespace ff {

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = std::float_t;
using f64 = std::double_t;


template<typename T>
using vec2  = glm::vec<2, T>;
using vec2u = glm::vec<2, u32>;
using vec2i = glm::vec<2, i32>;
using vec2f = glm::vec<2, f32>;
using vec2d = glm::vec<2, f64>;

template<typename T>
using vec3  = glm::vec<3, T>;
using vec3u = glm::vec<3, u32>;
using vec3i = glm::vec<3, i32>;
using vec3f = glm::vec<3, f32>;
using vec3d = glm::vec<3, f64>;

template<typename T>
using vec4  = glm::vec<4, T>;
using vec4u = glm::vec<4, u32>;
using vec4i = glm::vec<4, i32>;
using vec4f = glm::vec<4, f32>;
using vec4d = glm::vec<4, f64>;


template<typename T>
using mat3  = glm::mat<3, 3, T>;
using mat3u = glm::mat<3, 3, u32>;
using mat3i = glm::mat<3, 3, i32>;
using mat3f = glm::mat<3, 3, f32>;
using mat3d = glm::mat<3, 3, f64>;

template<typename T>
using mat4  = glm::mat<4, 4, T>;
using mat4u = glm::mat<4, 4, u32>;
using mat4i = glm::mat<4, 4, i32>;
using mat4f = glm::mat<4, 4, f32>;
using mat4d = glm::mat<4, 4, f64>;

}
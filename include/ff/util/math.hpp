#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/string_cast.hpp>
#undef GLM_ENABLE_EXPERIMENTAL
#else
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/string_cast.hpp>
#endif

namespace ff {

using namespace glm;

template<class T>
struct type_value;

template<> struct type_value<i8>  { constexpr static auto value = 0x1400; };
template<> struct type_value<u8>  { constexpr static auto value = 0x1401; };
template<> struct type_value<i16> { constexpr static auto value = 0x1402; };
template<> struct type_value<u16> { constexpr static auto value = 0x1403; };
template<> struct type_value<i32> { constexpr static auto value = 0x1404; };
template<> struct type_value<u32> { constexpr static auto value = 0x1405; };
template<> struct type_value<f32> { constexpr static auto value = 0x1406; };
template<> struct type_value<f64> { constexpr static auto value = 0x140A; };

template<class T>
constexpr static auto type_value_v = type_value<T>::value;

}
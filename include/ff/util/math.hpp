#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>

namespace ff {

template<typename T>
using vec2  = glm::vec<2, T>;
using vec2u = glm::vec<2, unsigned>;
using vec2i = glm::vec<2, int>;
using vec2f = glm::vec<2, float>;
using vec2d = glm::vec<2, double>;


template<typename T>
using vec3  = glm::vec<3, T>;
using vec3u = glm::vec<3, unsigned>;
using vec3i = glm::vec<3, int>;
using vec3f = glm::vec<3, float>;
using vec3d = glm::vec<3, double>;


template<typename T>
using vec4  = glm::vec<4, T>;
using vec4u = glm::vec<4, unsigned>;
using vec4i = glm::vec<4, int>;
using vec4f = glm::vec<4, float>;
using vec4d = glm::vec<4, double>;

template<typename T>
using mat3  = glm::mat<3, 3, T>;
using mat3u = glm::mat<3, 3, unsigned>;
using mat3i = glm::mat<3, 3, int>;
using mat3f = glm::mat<3, 3, float>;
using mat3d = glm::mat<3, 3, double>;

template<typename T>
using mat4  = glm::mat<4, 4, T>;
using mat4u = glm::mat<4, 4, unsigned>;
using mat4i = glm::mat<4, 4, int>;
using mat4f = glm::mat<4, 4, float>;
using mat4d = glm::mat<4, 4, double>;

}
#pragma once

#include <glm/glm.hpp>

namespace ff
{
    using Vec2f = glm::fvec2;
    using Vec3f = glm::fvec3;
    using Vec4f = glm::fvec4;

    using Vec2i = glm::ivec2;
    using Vec3i = glm::ivec3;
    using Vec4i = glm::ivec4;

    using Vec2u = glm::uvec2;
    using Vec3u = glm::uvec3;
    using Vec4u = glm::uvec4;

    template<typename T> using Vec2 = glm::tvec2<T>;
    template<typename T> using Vec3 = glm::tvec3<T>;
    template<typename T> using Vec4 = glm::tvec4<T>;
}

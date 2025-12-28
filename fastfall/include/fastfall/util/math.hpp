#pragma once

#include <regex>

#include "fastfall/util/glm_types.hpp"
#include "fastfall/util/Line.hpp"
#include "fastfall/util/Angle.hpp"
#include "fastfall/util/Rect.hpp"
#include "fastfall/util/direction.hpp"

#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtx/projection.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtx/closest_point.hpp"

namespace ff::math
{

    template<typename T>
    constexpr T epsilon()
    {
        return glm::epsilon<T>();
    }

    // VEC2

    template<typename T>
    constexpr Vec2<T> unit(const Vec2<T>& v)
    {
        auto r = glm::normalize(v);
        return !glm::any(glm::isnan(r)) ? r : Vec2<T>{};
    }

    template<typename T>
    Vec2<T> righthand_normal(const Vec2<T> &v)
    {
        return {-v.y, v.x};
    }

    template<typename T>
    Vec2<T> righthand_unit_normal(const Vec2<T> &v)
    {
        return unit(righthand_normal(v));
    }

    template<typename T>
    Vec2<T> lefthand_normal(const Vec2<T> &v)
    {
        return {v.y, -v.x};
    }

    template<typename T>
    Vec2<T> lefthand_unit_normal(const Vec2<T> &v)
    {
        return unit(lefthand_normal(v));
    }

    template<typename T>
    bool is_vertical(const Vec2<T> &v)
    {
        return v.x == 0;
    }

    template<typename T>
    bool is_horizontal(const Vec2<T> &v)
    {
        return v.y == 0;
    }

    template<typename T>
    constexpr auto mag(const Vec2<T>& v)
    {
        return glm::length(v);
    }

    template<typename T>
    constexpr auto mag2(const Vec2<T>& v)
    {
        return glm::length2(v);
    }

    template<typename T>
    constexpr bool is_unit(const Vec2<T>& v)
    {
        return glm::length2(v) == T{1};
    }

    template<typename T>
    constexpr auto dot(const Vec2<T>& v1, const Vec2<T>& v2)
    {
        return glm::dot(v1, v2);
    }

    template<typename T>
    constexpr auto proj(const Vec2<T>& v, const Vec2<T>& normal)
    {
        return glm::proj(v, normal);
    }

    template<typename T>
    constexpr bool collinear(const Vec2<T>& unit_v1, const Vec2<T>& unit_v2)
    {
        return std::abs(dot(unit_v1, unit_v2)) > (T{1} - glm::epsilon<T>());
    }

    template<typename T>
    constexpr auto rotate(const Vec2<T>& v, const Angle& ang)
    {
        return Vec2<T>{
            v.x * (T)std::cos(ang.radians()) - v.y * (T)std::sin(ang.radians()),
            v.x * (T)std::sin(ang.radians()) + v.y * (T)std::cos(ang.radians())
        };
    }

    template<typename T>
    constexpr auto dist(const Vec2<T>& v1, const Vec2<T>& v2)
    {
        return glm::distance(v1, v2);
    }

    template<typename T>
    constexpr auto dist2(const Vec2<T>& v1, const Vec2<T>& v2)
    {
        return glm::distance2(v1, v2);
    }

    template<typename T>
    constexpr Angle angle(const Vec2<T>& normal)
    {
        return Angle(std::atan2(normal.y, normal.x));
    }

    template<typename T, typename I>
    constexpr auto lerp(const Vec2<T>& v1, const Vec2<T>& v2, const I& t)
    {
        return glm::mix(v1, v2, t);
    }

    // LINE
    template<typename T>
    constexpr Line<T> reverse(const Line<T>& line) {
        return Line(line.p2, line.p1);
    }

    template<typename T>
    constexpr Vec2<T> midpoint(const Line<T>& line) {
        return glm::mix(line.p1, line.p2, 0.5);
    }

    template<typename T>
    constexpr Line<T> shift(const Line<T>& line, const Vec2<T>& offset) {
        return Line(line.p1 + offset, line.p2 + offset);
    }

    template<typename T>
    constexpr Vec2<T> vector(const Line<T>& line) {
        return line.p2 - line.p1;
    }

    template<typename T>
    constexpr Vec2<T> unit(const Line<T>& line) {
        return math::unit(math::vector(line));
    }

    template<typename T>
    constexpr bool is_vertical(const Line<T> &line)
    {
        return line.p1.x == line.p2.x;
    }

    template<typename T>
    constexpr bool is_horizontal(const Line<T> &line)
    {
        return line.p1.y == line.p2.y;
    }
    template<typename T>
    constexpr Angle angle(const Line<T>& line)
    {
        return angle(math::vector(line));
    }

    template<typename T>
    constexpr auto righthand_normal(const Line<T> &line)
    {
        return righthand_normal(math::vector(line));
    }

    template<typename T>
    constexpr auto righthand_unit_normal(const Line<T> &line)
    {
        return righthand_unit_normal(math::vector(line));
    }

    template<typename T>
    constexpr auto lefthand_normal(const Line<T> &line)
    {
        return lefthand_normal(math::vector(line));
    }

    template<typename T>
    constexpr auto lefthand_unit_normal(const Line<T> &line)
    {
        return lefthand_unit_normal(math::vector(line));
    }

    template<typename T>
    constexpr std::optional<Vec2<T>> intersection(const Line<T>& a, const Line<T>& b)
    {
        auto detA = (double)a.p1.x * (double)a.p2.y - (double)a.p1.y * (double)a.p2.x;
        auto detB = (double)b.p1.x * (double)b.p2.y - (double)b.p1.y * (double)b.p2.x;

        auto mxA = (double)a.p1.x - (double)a.p2.x;
        auto mxB = (double)b.p1.x - (double)b.p2.x;

        auto myA = (double)a.p1.y - (double)a.p2.y;
        auto myB = (double)b.p1.y - (double)b.p2.y;

        auto xnom = detA * mxB - detB * mxA;
        auto ynom = detA * myB - detB * myA;

        auto denom = mxA * myB - myA * mxB;

        if (denom == static_cast<T>(0)) {
            // is parallel
            return std::nullopt;
        }

        auto ixOut = xnom / denom;
        auto iyOut = ynom / denom;

        if constexpr (std::is_floating_point_v<T>)
        {
            if (!std::isfinite(ixOut) || !std::isfinite(iyOut)) {
                //numerical issue?
                return std::nullopt;
            }
        }

        return Vec2<T>{ ixOut, iyOut };
    }

    template<typename T>
    constexpr Vec2<T> closest_point_on_line(const Line<T>& line, const Vec2<T>& point)
    {
        auto result3 = glm::closestPointOnLine(
                Vec3<T>{point.x, point.y, 0},
                Vec3<T>{line.p1.x, line.p1.y, 0},
                Vec3<T>{line.p2.x, line.p2.y, 0}
            );
        Vec2<T> result{ result3 };
        return result;
    }

    template<typename T>
    constexpr T distance_from_line(const Line<T>& line, const Vec2<T>& point)
    {
        auto result3 = glm::closestPointOnLine(
                Vec3<T>{point.x, point.y, 0},
                Vec3<T>{line.p1.x, line.p1.y, 0},
                Vec3<T>{line.p2.x, line.p2.y, 0}
            );
        Vec2<T> result{ result3 };
        return math::dist(result, point);
    }

    template<typename T>
    constexpr bool collinear(const Line<T>& a, const Line<T>& b)
    {
        return collinear(unit(a.p2 - a.p1), unit(b.p2 - a.p1));
    }

    template<typename T>
    constexpr auto dist(const Line<T>& line)
    {
        return glm::distance(line.p1, line.p2);
    }

    template<typename T>
    constexpr auto dist2(const Line<T>& line)
    {
        return glm::distance2(line.p1, line.p2);
    }

    // RECT
    template<typename T>
    [[nodiscard]] constexpr Rect<T> rect_extend(const Rect<T>& a, Cardinal dir, T amount) {
        Rect<T> out = a;
        T temp;
        switch (dir) {
        case Cardinal::N:
            temp = out.top + out.height;
            out.top -= amount;
            out.height = temp - out.top;
            break;
        case Cardinal::E:
            out.width += amount;
            break;
        case Cardinal::S:
            out.height += amount;
            break;
        case Cardinal::W:
            temp = out.left + out.width;
            out.left -= amount;
            out.width = temp - out.left;
            break;
        }
        return out;
    }

    template<typename T>
    constexpr Rect<T> shift(const Rect<T>& rect, const Vec2<T>& offset) {
        return Rect(rect.getPosition() + offset, rect.getSize());
    }

    // BOUNDS

    template<typename T>
    [[nodiscard]]
    Rect<T> line_bounds(const Line<T>& a) {
        Rect<T> line_bounds;
        line_bounds.left   = std::min(a.p1.x, a.p2.x);
        line_bounds.top    = std::min(a.p1.y, a.p2.y);
        line_bounds.width  = std::max(a.p1.x, a.p2.x) - line_bounds.left;
        line_bounds.height = std::max(a.p1.y, a.p2.y) - line_bounds.top;
        return line_bounds;
    }

    template<typename T>
    [[nodiscard]]
    Rect<T> rect_bounds(const Rect<T>& a, const Rect<T>& b) {
        Rect<T> r;
        r.left = std::min(a.left, b.left);
        r.top = std::min(a.top, b.top);
        r.width = std::max(a.left + a.width, b.left + b.width) - r.left;
        r.height = std::max(a.top + a.height, b.top + b.height) - r.top;
        return r;
    }

    // ANGLE

    template<typename T = float>
    constexpr Vec2<T> vector(const Angle& ang) {
        return Vec2<T>{
            std::cos(ang.radians()),
            std::sin(ang.radians()),
        };
    }

    template<typename T = float>
    constexpr Vec2<T> unit(const Angle& ang)
    {
        return Vec2<T>{
            std::cos(ang.radians()),
            std::sin(ang.radians()),
        };
    }

}

namespace fmt {

    template<glm::length_t L, typename T, glm::qualifier Q>
    struct formatter<glm::vec<L, T, Q>> {

        // Parse the format string to extract the precision
        template<typename ParseContext>
        constexpr auto parse(ParseContext &ctx) {
            auto it = ctx.begin();
            auto end = ctx.end();

            while (it != end && *it != '}')
            {
                ++it;  // Skip to the end of the range
            }

            // Return the iterator after parsing
            return it;
        }

        template<typename Scalar, typename FormatContext>
        constexpr auto format_element(Scalar value, FormatContext &ctx) const {
            fmt::format_to(ctx.out(), "{}", value);
        }

        // Format the matrix with the specified precision
        template<typename FormatContext>
        constexpr auto format(const glm::vec<L, T, Q>& v, FormatContext &ctx) const {
            fmt::format_to(ctx.out(), "{}", "(");
            for (int i = 0; i < L; i++)
            {
                format_element(v[i], ctx);
                if (i != L - 1)
                    fmt::format_to(ctx.out(), "{}", ", ");
            }
            fmt::format_to(ctx.out(), "{}", ")");
            return ctx.out();
        }
    };

}

namespace glm
{
    template<typename T, qualifier Q>
    requires std::is_integral_v<T>
    constexpr std::strong_ordering operator<=> (const vec<2, T, Q>& a, const vec<2, T, Q>& b)
    {
        if (a.y == b.y)
        {
            if (a.x < b.x) return std::strong_ordering::less;
            if (a.x > b.x) return std::strong_ordering::greater;
        }
        if (a.y < b.y) return std::strong_ordering::less;
        if (a.y > b.y) return std::strong_ordering::greater;
        return std::strong_ordering::equal;
    }

    template<typename T, qualifier Q>
    requires std::is_integral_v<T>
    constexpr bool operator<(const vec<2, T, Q>& a, const vec<2, T, Q>& b)
    {
        return a.y == b.y ? a.x < b.x : a.y < b.y;
    }
}

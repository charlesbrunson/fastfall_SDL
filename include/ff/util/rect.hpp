#pragma once

#include "line.hpp"

#include <glm/vec2.hpp>

namespace ff {

template<typename T>
struct rect {
    constexpr rect()
    {
        left = 0;
        top = 0;
        width = 0;
        height = 0;
    }

    constexpr rect(T r_left, T r_top, T r_width, T r_height)
    {
        left = r_left;
        top = r_top;
        width = r_width;
        height = r_height;
    }

    constexpr rect(glm::vec<2, T> topleft, glm::vec<2, T> size)
    {
        left = topleft.x;
        top = topleft.y;
        width = size.x;
        height = size.y;
    }

    template<typename U>
    constexpr rect(const rect<U>& rect)
    {
        left = static_cast<T>(rect.left);
        top = static_cast<T>(rect.top);
        width = static_cast<T>(rect.width);
        height = static_cast<T>(rect.height);
    }

    inline void set_position(T X, T Y)
    {
        left = X; top = Y;
    }
    inline void set_position(glm::vec<2, T> position)
    {
        left = position.x; top = position.y;
    }
    inline void set_size(T W, T H)
    {
        width = W; height = H;
    }
    inline void set_size(glm::vec<2, T> size)
    {
        width = size[0]; height = size[1];
    }

    inline glm::vec<2, T> get_position() const
    {
        return glm::vec<2, T>{left, top};
    };
    inline glm::vec<2, T> get_size() const
    {
        return glm::vec<2, T>{width, height};
    };
    inline T getArea() const
    {
        return width * height;
    };

    rect<T>& operator* (T value) {
        left	*= value;
        top		*= value;
        width	*= value;
        height	*= value;
        return *this;
    }
    rect<T>& operator/ (T value) {
        left	/= value;
        top		/= value;
        width	/= value;
        height	/= value;
        return *this;
    }

    bool operator== (const rect<T> rect) const
    {
        return left == rect.left
               && top == rect.top
               && width == rect.width
               && height == rect.height;

    }

    constexpr vec4<T> as_vec4() const {
        return {
                left,
                top,
                width,
                height
        };
    }

    bool intersects(const rect<T>& rectangle) const
    {
        rect<T> intersection;
        return intersects(rectangle, intersection);
    }

    bool intersects(const rect<T>& rectangle, rect<T>& intersection) const
    {
        // Rectangles with negative dimensions are allowed, so we must handle them correctly

        // Compute the min and max of the first rectangle on both axes
        T r1MinX = std::min(left, static_cast<T>(left + width));
        T r1MaxX = std::max(left, static_cast<T>(left + width));
        T r1MinY = std::min(top, static_cast<T>(top + height));
        T r1MaxY = std::max(top, static_cast<T>(top + height));

        // Compute the min and max of the second rectangle on both axes
        T r2MinX = std::min(rectangle.left, static_cast<T>(rectangle.left + rectangle.width));
        T r2MaxX = std::max(rectangle.left, static_cast<T>(rectangle.left + rectangle.width));
        T r2MinY = std::min(rectangle.top, static_cast<T>(rectangle.top + rectangle.height));
        T r2MaxY = std::max(rectangle.top, static_cast<T>(rectangle.top + rectangle.height));

        // Compute the intersection boundaries
        T interLeft   = std::max(r1MinX, r2MinX);
        T interTop    = std::max(r1MinY, r2MinY);
        T interRight  = std::min(r1MaxX, r2MaxX);
        T interBottom = std::min(r1MaxY, r2MaxY);

        // If the intersection is valid (positive non zero area), then there is an intersection
        if ((interLeft < interRight) && (interTop < interBottom))
        {
            intersection = rect<T>(interLeft, interTop, interRight - interLeft, interBottom - interTop);
            return true;
        }
        else
        {
            intersection = rect<T>(0, 0, 0, 0);
            return false;
        }
    }


    bool touches(const rect<T>& rectangle) const
    {
        rect<T> intersection;
        return touches(rectangle, intersection);
    }

    bool touches(const rect<T>& rectangle, rect<T>& intersection) const
    {
        // Compute the min and max of the first rectangle on both axes
        T r1MinX = std::min(left, static_cast<T>(left + width));
        T r1MaxX = std::max(left, static_cast<T>(left + width));
        T r1MinY = std::min(top, static_cast<T>(top + height));
        T r1MaxY = std::max(top, static_cast<T>(top + height));

        // Compute the min and max of the second rectangle on both axes
        T r2MinX = std::min(rectangle.left, static_cast<T>(rectangle.left + rectangle.width));
        T r2MaxX = std::max(rectangle.left, static_cast<T>(rectangle.left + rectangle.width));
        T r2MinY = std::min(rectangle.top, static_cast<T>(rectangle.top + rectangle.height));
        T r2MaxY = std::max(rectangle.top, static_cast<T>(rectangle.top + rectangle.height));

        // Compute the intersection boundaries
        T interLeft = std::max(r1MinX, r2MinX);
        T interTop = std::max(r1MinY, r2MinY);
        T interRight = std::min(r1MaxX, r2MaxX);
        T interBottom = std::min(r1MaxY, r2MaxY);

        if ((interLeft <= interRight) && (interTop <= interBottom))
        {
            intersection = Rect<T>(interLeft, interTop, interRight - interLeft, interBottom - interTop);
            return true;
        }
        else
        {
            intersection = Rect<T>(0, 0, 0, 0);
            return false;
        }
    }

    bool contains(const glm::vec<2, T>& pos) const
    {
        T MinX = std::min(left, static_cast<T>(left + width));
        T MaxX = std::max(left, static_cast<T>(left + width));
        T MinY = std::min(top,  static_cast<T>(top + height));
        T MaxY = std::max(top,  static_cast<T>(top + height));

        return pos.x >= MinX && pos.x <= MaxX
               && pos.y >= MinY && pos.y <= MaxY;
    }

    bool contains(const line<T>& line) const
    {
        T minX = std::min(left, static_cast<T>(left + width));
        T maxX = std::max(left, static_cast<T>(left + width));
        T minY = std::min(top,  static_cast<T>(top + height));
        T maxY = std::max(top,  static_cast<T>(top + height));

        if ((line.p1.x < minX && line.p2.x < minX)
            || (line.p1.y < minY && line.p2.y < minY)
            || (line.p1.x > maxX && line.p2.x > maxX)
            || (line.p1.y > maxY && line.p2.y > maxY))
            return false;

        float m = (line.p2.y - line.p1.y) / (line.p2.x - line.p1.x);

        float y = m * (minX - line.p1.x) + line.p1.y;
        if (y >= minY && y <= maxY) return true;

        y = m * (maxX - line.p1.x) + line.p1.y;
        if (y >= minY && y <= maxY) return true;

        float x = (minY - line.p1.y) / m + line.p1.x;
        if (x >= minX && x <= maxX) return true;

        x = (maxY - line.p1.y) / m + line.p1.x;
        if (x >= minX && x <= maxX) return true;

        return false;
    }

    [[nodiscard]] constexpr glm::vec<2, T> topleft () const { return glm::vec<2, T>(left,                top); }
    [[nodiscard]] constexpr glm::vec<2, T> topmid  () const { return glm::vec<2, T>(left + width / T{2}, top); }
    [[nodiscard]] constexpr glm::vec<2, T> topright() const { return glm::vec<2, T>(left + width,        top); }
    [[nodiscard]] constexpr glm::vec<2, T> leftmid () const { return glm::vec<2, T>(left,                top + height / T{2}); }
    [[nodiscard]] constexpr glm::vec<2, T> center  () const { return glm::vec<2, T>(left + width / T{2}, top + height / T{2}); }
    [[nodiscard]] constexpr glm::vec<2, T> rightmid() const { return glm::vec<2, T>(left + width,        top + height / T{2}); }
    [[nodiscard]] constexpr glm::vec<2, T> botright() const { return glm::vec<2, T>(left + width,        top + height); }
    [[nodiscard]] constexpr glm::vec<2, T> botmid  () const { return glm::vec<2, T>(left + width / T{2}, top + height); }
    [[nodiscard]] constexpr glm::vec<2, T> botleft () const { return glm::vec<2, T>(left,                top + height); }

    T left;
    T top;
    T width;
    T height;
};

using rectu = rect<unsigned>;
using recti = rect<int>;
using rectf = rect<float>;
using rectd = rect<double>;

}
#pragma once

#include <array>

#include "fastfall/util/glm_types.hpp"
#include "fastfall/util/Line.hpp"

namespace ff {

template<typename T>
class Rect {
public:
	using value_type = T;
	using point_type = Vec2<T>;
	using size_type  = Vec2<T>;

	constexpr Rect() 
	{
		left = 0;
		top = 0;
		width = 0;
		height = 0;
	}

	constexpr Rect(T r_left, T r_top, T r_width, T r_height)
	{
		left = r_left;
		top = r_top;
		width = r_width;
		height = r_height;
	}

	constexpr Rect(point_type topleft, size_type size)
	{
		left = topleft.x;
		top = topleft.y;
		width = size.x;
		height = size.y;
	}

	template<typename U>
	constexpr Rect(const Rect<U>& rect)
	{
		left = static_cast<T>(rect.left);
		top = static_cast<T>(rect.top);
		width = static_cast<T>(rect.width);
		height = static_cast<T>(rect.height);
	}

	void setPosition(T X, T Y)
	{
		left = X; top = Y;
	}
	void setPosition(point_type position)
	{
		left = position[0]; top = position[1];
	}

	void setSize(T W, T H)
	{
		width = W; height = H;
	}
	void setSize(size_type size)
	{
		width = size[0]; height = size[1];
	}

	point_type getPosition() const
	{ 
		return point_type{left, top};
	}
	point_type getSize() const
	{ 
		return point_type{width, height};
	}
    T getArea() const
    {
        return width * height;
    }

	Rect operator* (T value) {
		return {
			left	* value,
			top		* value,
			width	* value,
			height	* value,
		};
	}
	Rect operator/ (T value) {
		return {
			left	/ value,
			top		/ value,
			width	/ value,
			height	/ value,
		};
	}

	bool operator== (const Rect rect) const
	{
		return left == rect.left
			&& top == rect.top
			&& width == rect.width
			&& height == rect.height;
	}

	constexpr Vec4<T> toVec4() const {
		return {
			left,
			top,
			width,
			height
		};
	}

	constexpr std::array<point_type, 4> toPoints() const {
		std::array<point_type, 4> arr;

		T right = left + width;
		T bottom = top + height;

		arr[0] = { left, top };
		arr[1] = { right, top };
		arr[2] = { left, bottom };
		arr[3] = { right, bottom };
		return arr;
	}

	bool intersects(const Rect& rectangle) const
	{
		Rect tmp;
		return intersects(rectangle, tmp);
	}

	bool intersects(const Rect& rectangle, Rect& intersection) const
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
			intersection = Rect(interLeft, interTop, interRight - interLeft, interBottom - interTop);
			return true;
		}
		else
		{
			intersection = Rect(0, 0, 0, 0);
			return false;
		}
	}


	bool touches(const Rect& rectangle) const
	{
		Rect tmp;
		return touches(rectangle, tmp);
	}

	bool touches(const Rect& rectangle, Rect& intersection) const
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
			intersection = Rect(interLeft, interTop, interRight - interLeft, interBottom - interTop);
			return true;
		}
		else
		{
			intersection = Rect(0, 0, 0, 0);
			return false;
		}
	}

	bool contains(const Vec2<T>& pos) const
	{
		T MinX = std::min(left, static_cast<T>(left + width));
		T MaxX = std::max(left, static_cast<T>(left + width));
		T MinY = std::min(top,  static_cast<T>(top + height));
		T MaxY = std::max(top,  static_cast<T>(top + height));

		return pos.x >= MinX && pos.x <= MaxX
			&& pos.y >= MinY && pos.y <= MaxY;
	}

    bool contains(const Line<T>& line) const
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

    [[nodiscard]] constexpr Vec2<T> topleft () const { return Vec2<T>(left,                top); }
    [[nodiscard]] constexpr Vec2<T> topmid  () const { return Vec2<T>(left + width / T{2}, top); }
    [[nodiscard]] constexpr Vec2<T> topright() const { return Vec2<T>(left + width,        top); }
    [[nodiscard]] constexpr Vec2<T> leftmid () const { return Vec2<T>(left,                top + height / T{2}); }
    [[nodiscard]] constexpr Vec2<T> center  () const { return Vec2<T>(left + width / T{2}, top + height / T{2}); }
    [[nodiscard]] constexpr Vec2<T> rightmid() const { return Vec2<T>(left + width,        top + height / T{2}); }
    [[nodiscard]] constexpr Vec2<T> botright() const { return Vec2<T>(left + width,        top + height); }
    [[nodiscard]] constexpr Vec2<T> botmid  () const { return Vec2<T>(left + width / T{2}, top + height); }
    [[nodiscard]] constexpr Vec2<T> botleft () const { return Vec2<T>(left,                top + height); }

	T left;
	T top;
	T width;
	T height;

};

typedef Rect<float> Rectf;
typedef Rect<int> Recti;
typedef Rect<unsigned> Rectu;

}
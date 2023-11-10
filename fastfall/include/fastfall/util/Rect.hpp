#pragma once

#include <array>

#include "glm/glm.hpp"
#include "fastfall/util/Vec2.hpp"

namespace ff {

template<typename T>
class Rect {
public:
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

	constexpr Rect(glm::vec<2, T> topleft, glm::vec<2, T> size)
	{
		left = topleft.x;
		top = topleft.y;
		width = size.x;
		height = size.y;
	}

	constexpr Rect(Vec2<T> topleft, Vec2<T> size)
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

	inline void setPosition(T X, T Y) 
	{
		left = X; top = Y;
	}
	inline void setPosition(glm::vec<2, T> position) 
	{
		left = position[0]; top = position[1];
	}
	inline void setPosition(Vec2<T> position)
	{
		left = position.x; top = position.y;
	}
	inline void setSize(T W, T H)
	{
		width = W; height = H;
	}
	inline void setSize(glm::vec<2, T> size)
	{
		width = size[0]; height = size[1];
	}
	inline void setSize(Vec2<T> size)
	{
		width = size[0]; height = size[1];
	}

	inline glm::vec<2, T> getPosition() const 
	{ 
		return glm::vec<2, T>{left, top}; 
	};
	inline glm::vec<2, T> getSize() const 
	{ 
		return glm::vec<2, T>{width, height}; 
	};
    inline T getArea() const
    {
        return width * height;
    };

	Rect<T>& operator* (T value) {
		left	*= value;
		top		*= value;
		width	*= value;
		height	*= value;
		return *this;
	}
	Rect<T>& operator/ (T value) {
		left	/= value;
		top		/= value;
		width	/= value;
		height	/= value;
		return *this;
	}

	bool operator== (const Rect<T> rect) const
	{
		return left == rect.left
			&& top == rect.top
			&& width == rect.width
			&& height == rect.height;

	}

	constexpr glm::vec<4, T> toVec4() const {
		return glm::vec<4, T>{
				left,
				top,
				width,
				height
			};
	}


	constexpr std::array<glm::vec<2, T>, 4> toPoints() const {
		std::array<glm::vec<2, T>, 4> arr;

		T right = left + width;
		T bottom = top + height;

		arr[0] = { left, top };
		arr[1] = { right, top };
		arr[2] = { left, bottom };
		arr[3] = { right, bottom };
		return arr;
	}


	bool intersects(const Rect<T>& rectangle) const
	{
		Rect<T> intersection;
		return intersects(rectangle, intersection);
	}

	bool intersects(const Rect<T>& rectangle, Rect<T>& intersection) const
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
			intersection = Rect<T>(interLeft, interTop, interRight - interLeft, interBottom - interTop);
			return true;
		}
		else
		{
			intersection = Rect<T>(0, 0, 0, 0);
			return false;
		}
	}


	bool touches(const Rect<T>& rectangle) const
	{
		Rect<T> intersection;
		return touches(rectangle, intersection);
	}

	bool touches(const Rect<T>& rectangle, Rect<T>& intersection) const
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

	bool contains(const Vec2<T>& pos) const
	{
		T MinX = std::min(left, static_cast<T>(left + width));
		T MaxX = std::max(left, static_cast<T>(left + width));
		T MinY = std::min(top,  static_cast<T>(top + height));
		T MaxY = std::max(top,  static_cast<T>(top + height));

		return pos.x >= MinX && pos.x <= MaxX
			&& pos.y >= MinY && pos.y <= MaxY;
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
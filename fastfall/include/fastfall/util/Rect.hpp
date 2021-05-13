#pragma once

#include <array>

#include "glm/glm.hpp"

namespace ff {

template<typename T>
class Rect {
public:
	Rect() 
	{
		left = 0;
		top = 0;
		width = 0;
		height = 0;
	}

	Rect(T r_left, T r_top, T r_width, T r_height)
	{
		left = r_left;
		top = r_top;
		width = r_width;
		height = r_height;
	}

	Rect(glm::vec<2, T> topleft, glm::vec<2, T> size) 
	{
		left = topleft.x;
		top = topleft.y;
		width = size.x;
		height = size.y;
	}

	inline void setPosition(T X, T Y) 
	{
		left = X; top = Y;
	}
	inline void setPosition(glm::vec<2, T> position) 
	{
		left = position[0]; top = position[1];
	}
	inline void setSize(T W, T H)
	{
		width = W; height = H;
	}
	inline void setSize(glm::vec<2, T> size)
	{
		left = size[0]; top = size[1];
	}

	inline glm::vec<2, T> getPosition() const 
	{ 
		return glm::vec<2, T>{left, top}; 
	};
	inline glm::vec<2, T> getSize() const 
	{ 
		return glm::vec<2, T>{width, height}; 
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

	glm::vec<4, T> toVec4() const {
		return glm::vec<4, T>{
				left,
				top,
				width,
				height
			};
	}

	std::array<glm::vec<2, T>, 4> toPoints() const {
		std::array<glm::vec<2, T>, 4> arr;

		T right = left + width;
		T bottom = top + height;

		arr[0] = { left, top };
		arr[1] = { right, top };
		arr[2] = { left, bottom };
		arr[3] = { right, bottom };
		return arr;
	}

	T left;
	T top;
	T width;
	T height;

};

typedef Rect<float> Rectf;
typedef Rect<int> Recti;
typedef Rect<unsigned> Rectu;

}
#pragma once

//#include <SFML/System.hpp>

#include <stdexcept>

#include "fastfall/util/Vec2.hpp"

namespace ff {

template<class T>
class Line {
public:
	constexpr Line() :
		p1(),
		p2()
	{

	};
	constexpr Line(Vec2<T> point1, Vec2<T> point2) :
		p1(point1),
		p2(point2)
	{

	};

	template<class Y>
	constexpr Line(Line<Y>& line) :
		p1(line.p1),
		p2(line.p2)
	{

	};

	Vec2<T> p1;
	Vec2<T> p2;

	constexpr inline Vec2<T>& operator[](std::size_t ndx) {
		if (ndx >= 2)
			throw std::out_of_range("out of range");

		return ndx == 0 ? p1 : p2;
	};
	constexpr inline const Vec2<T>& operator[](std::size_t ndx) const {
		if (ndx >= 2)
			throw std::out_of_range("out of range");

		return ndx == 0 ? p1 : p2;
	};

	std::string to_string() const noexcept {
		return p1.to_string() + "->" + p2.to_string();
	}
};

typedef Line<int> Linei;
typedef Line<float> Linef;

}

template<class T>
inline bool operator==(const ff::Line<T>& lhs, const ff::Line<T>& rhs) {
	return (lhs.p1 == rhs.p1) && (lhs.p2 == rhs.p2);
}

template<class T>
inline bool operator!=(const ff::Line<T>& lhs, const ff::Line<T>& rhs) {
	return (lhs.p1 != rhs.p1) || (lhs.p2 != rhs.p2);
}

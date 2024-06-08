#pragma once

#include <stdexcept>

#include "math.hpp"

namespace ff {

template<class T>
class line {
public:
	constexpr line() :
		p1(),
		p2()
	{

	};
	constexpr line(vec2<T> point1, vec2<T> point2) :
		p1(point1),
		p2(point2)
	{

	};

	template<class Y>
	constexpr line(line<Y>& line) :
		p1(line.p1),
		p2(line.p2)
	{

	};

	vec2<T> p1;
	vec2<T> p2;

	constexpr inline vec2<T>& operator[](std::size_t ndx) {
		if (ndx >= 2)
			throw std::out_of_range("out of range");

		return ndx == 0 ? p1 : p2;
	};
	constexpr inline const vec2<T>& operator[](std::size_t ndx) const {
		if (ndx >= 2)
			throw std::out_of_range("out of range");

		return ndx == 0 ? p1 : p2;
	};

	std::string to_string() const noexcept {
		return p1.to_string() + "->" + p2.to_string();
	}

	line<T> reverse() const {
		return line<T>(p2, p1);
	}

};

using linei = line<int>;
using linef = line<float>;

}

template<class T>
inline bool operator==(const ff::line<T>& lhs, const ff::line<T>& rhs) {
	return (lhs.p1 == rhs.p1) && (lhs.p2 == rhs.p2);
}

template<class T>
inline bool operator!=(const ff::line<T>& lhs, const ff::line<T>& rhs) {
	return (lhs.p1 != rhs.p1) || (lhs.p2 != rhs.p2);
}

#pragma once

#include <stdexcept>

#include "fastfall/util/glm_types.hpp"

namespace ff {

template<class T>
class Line {
public:
	using value_type = T;
	using point_type = Vec2<T>;

	constexpr Line() :
		p1(),
		p2()
	{

	};
	constexpr Line(point_type point1, point_type point2) :
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

	point_type p1;
	point_type p2;

	constexpr point_type& operator[](std::size_t ndx) {
		if (ndx >= 2)
			throw std::out_of_range("out of range");

		return ndx == 0 ? p1 : p2;
	};
	constexpr const point_type& operator[](std::size_t ndx) const {
		if (ndx >= 2)
			throw std::out_of_range("out of range");

		return ndx == 0 ? p1 : p2;
	};

	[[nodiscard]] std::string to_string() const noexcept {
		return p1.to_string() + "->" + p2.to_string();
	}

	/*
	constexpr Line reverse() const {
		return Line(p2, p1);
	}

	constexpr Line shift(const Vec2<T>& offset) const {
		return Line(p2 + offset, p1 + offset);
	}

	constexpr Vec2<T> vector() const
	{
		return p2 - p1;
	}
	*/

	friend bool operator==(const Line& lhs, const Line& rhs) {
		return (lhs.p1 == rhs.p1) && (lhs.p2 == rhs.p2);
	}

	friend bool operator!=(const Line& lhs, const Line& rhs) {
		return (lhs.p1 != rhs.p1) || (lhs.p2 != rhs.p2);
	}
};

using Linei = Line<int>;
using Linef = Line<float>;

}


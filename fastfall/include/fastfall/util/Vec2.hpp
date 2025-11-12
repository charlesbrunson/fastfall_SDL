#pragma once

#include <glm/vec2.hpp>

#include <type_traits>
#include <string>
#include <cmath>
#include "fmt/format.h"

namespace ff {

template<typename Type>
    requires std::is_arithmetic_v<Type>
class Vec2 {
public:
	constexpr Vec2() :
		x(0),
		y(0)
	{

	};

	constexpr Vec2(Type X, Type Y) :
		x(X),
		y(Y)
	{

	}

	constexpr Vec2<Type>& operator=(const glm::vec<2, Type>& other)
	{
		x = other.x;
		y = other.y;
		return *this;
	}
	
	template <typename T>
	constexpr explicit Vec2(const Vec2<T>& vector) :
		x(static_cast<Type>(vector.x)),
		y(static_cast<Type>(vector.y))
	{

	}

	constexpr Vec2(const glm::vec<2, Type>& vector) :
		x(vector.x),
		y(vector.y)
	{

	}

	//template <typename P>
	constexpr operator glm::vec<2, Type>() const {
		return glm::vec<2, Type>(x,	y);
	}

	Vec2<Type> lefthand() const {
		return Vec2<Type>(y, -x);
	}
	Vec2<Type> righthand() const {
		return Vec2<Type>(-y, x);
	}

	Vec2<Type> reverse() const {
		return Vec2<Type>(-x, -y);
	}

	Vec2<Type> abs() const {
		return Vec2<Type>(x < static_cast<Type>(0) ? -x : x, y < static_cast<Type>(0) ? -y : y);
	}

	template <typename T = Type, typename = std::enable_if_t<std::is_floating_point<T>::value>>
	Vec2<T> unit() const {
		T m = magnitude();
		if (m > 0.0)
			return Vec2<T>(x / m, y / m);
		else
			return Vec2<T>(static_cast<T>(0), static_cast<T>(0));
	}

	template <typename T = Type, typename = std::enable_if_t<std::is_floating_point<T>::value>>
	T magnitude() const {
		return sqrt(x * x + y * y);
	}

	Type magnitudeSquared() const {
		return x * x + y * y;
	}

	std::string to_string() const noexcept {
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
	}

	template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
	bool operator< (const ff::Vec2<T>& right) const noexcept {
		return y == right.y ?
			x < right.x :
			y < right.y;
	}


	Type x;
	Type y;
};

using Vec2i = Vec2<int>;
using Vec2u = Vec2<unsigned int>;
using Vec2f = Vec2<float>;

} // namespace ff

template <typename T>
constexpr inline ff::Vec2<T> operator-(const ff::Vec2<T>& right) {
	return ff::Vec2<T>(-right.x, -right.y);
}

template <typename T, typename U>
constexpr inline ff::Vec2<T>& operator+=(ff::Vec2<T>& left, const ff::Vec2<U>& right) {
	left.x += static_cast<T>(right.x);
	left.y += static_cast<T>(right.y);
	return left;
}

template <typename T, typename U>
constexpr inline ff::Vec2<T>& operator-=(ff::Vec2<T>& left, const ff::Vec2<U>& right) {
	left.x -= static_cast<T>(right.x);
	left.y -= static_cast<T>(right.y);
	return left;
}

template <typename T, typename U>
constexpr inline ff::Vec2<T> operator+(const ff::Vec2<T>& left, const ff::Vec2<U>& right) {
	return ff::Vec2<T>(left.x + static_cast<T>(right.x), left.y + static_cast<T>(right.y));
}

template <typename T, typename U>
constexpr inline ff::Vec2<T> operator-(const ff::Vec2<T>& left, const ff::Vec2<U>& right) {
	return ff::Vec2<T>(left.x - static_cast<T>(right.x), left.y - static_cast<T>(right.y));
}

template <typename T, typename U>
constexpr inline ff::Vec2<T> operator*(const ff::Vec2<T>& left, const U right) {
	return ff::Vec2<T>(left.x * static_cast<T>(right), left.y * static_cast<T>(right));
}

template <typename T, typename U>
constexpr inline ff::Vec2<U> operator*(const T left, const ff::Vec2<U>& right) {
	return ff::Vec2<U>(right.x * left, right.y * left);
}

template <typename T, typename U>
constexpr inline ff::Vec2<U> operator*(const ff::Vec2<T>& left, const ff::Vec2<U>& right) {
    return ff::Vec2<U>(right.x * left.x, right.y * left.y);
}

template <typename T, typename U>
constexpr inline ff::Vec2<T>& operator*=(ff::Vec2<T>& left, const U right) {
	left.x *= right;
	left.y *= right;
	return left;
}

template <typename T, typename U>
constexpr inline ff::Vec2<T>& operator*=(ff::Vec2<T>& left, const ff::Vec2<U>& right) {
    left.x *= right.x;
    left.y *= right.y;
    return left;
}

template <typename T, typename U>
constexpr inline ff::Vec2<T> operator/(const ff::Vec2<T>& left, const U right) {
	if (right == 0) throw std::runtime_error{ "Vec2: divide by zero" };
	return ff::Vec2<T>(left.x / static_cast<T>(right), left.y / static_cast<T>(right));
}

template <typename T, typename U>
constexpr inline ff::Vec2<T>& operator/=(ff::Vec2<T>& left, const U right) {
	if (right == 0) throw std::runtime_error{ "Vec2: divide by zero" };
	left.x /= right;
	left.y /= right;
	return left;
}

template <typename T>
constexpr inline bool operator==(const ff::Vec2<T>& left, const ff::Vec2<T>& right) noexcept {
	return (left.x == right.x) && (left.y == right.y);
}

template <typename T>
constexpr inline bool operator!=(const ff::Vec2<T>& left, const ff::Vec2<T>& right) noexcept {
	return (left.x != right.x) || (left.y != right.y);
}

/*
template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
inline bool operator< (const ff::Vec2<T>& left, const ff::Vec2<T>& right) noexcept {
	return left.y == right.y ?
		left.x < right.x :
		left.y < right.y;
}
*/

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
constexpr inline bool operator<= (const ff::Vec2<T>& left, const ff::Vec2<T>& right) noexcept {
	return left == right || left < right;
}

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
constexpr inline bool operator> (const ff::Vec2<T>& left, const ff::Vec2<T>& right) noexcept {
	return !(left <= right);
}

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
constexpr inline bool operator>= (const ff::Vec2<T>& left, const ff::Vec2<T>& right) noexcept {
	return !(left < right);
}


namespace fmt {
	template <typename T, typename Char>
	struct formatter<ff::Vec2<T>, Char> : formatter<T, Char> {
		template <typename FormatContext>
		constexpr auto format(const ff::Vec2<T>& p, FormatContext& ctx) const -> decltype(ctx.out()) {
			return format_to(ctx.out(), "({}, {})", p.x, p.y);
		}
	};
}

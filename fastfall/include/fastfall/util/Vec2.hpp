#pragma once

//#include <SFML/System.hpp>

#include <glm/vec2.hpp>

#include <type_traits>
#include <string>

namespace ff {

template<typename Type, typename = std::enable_if_t<std::is_arithmetic<Type>::value>>
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


	//template <typename P>
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


	//template <typename P>
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

	Type x;
	Type y;
};

typedef Vec2<int> Vec2i;
typedef Vec2<unsigned int> Vec2u;
typedef Vec2<float> Vec2f;

}

template <typename T>
inline ff::Vec2<T> operator -(const ff::Vec2<T>& right) {
	return Vec2<T>(-right.x, -right.y);
}

template <typename T, typename U>
inline ff::Vec2<T>& operator +=(ff::Vec2<T>& left, const ff::Vec2<U>& right) {
	left.x += static_cast<T>(right.x);
	left.y += static_cast<T>(right.y);
	return left;
}

template <typename T, typename U>
inline ff::Vec2<T>& operator -=(ff::Vec2<T>& left, const ff::Vec2<U>& right) {
	left.x -= static_cast<T>(right.x);
	left.y -= static_cast<T>(right.y);
	return left;
}

template <typename T, typename U>
inline ff::Vec2<T> operator +(const ff::Vec2<T>& left, const ff::Vec2<U>& right) {
	return Vec2<T>(left.x + static_cast<T>(right.x), left.y + static_cast<T>(right.y));
}

template <typename T, typename U>
inline ff::Vec2<T> operator -(const ff::Vec2<T>& left, const ff::Vec2<U>& right) {
	return Vec2<T>(left.x - static_cast<T>(right.x), left.y - static_cast<T>(right.y));
}

template <typename T, typename U>
inline ff::Vec2<T> operator *(const ff::Vec2<T>& left, U right) {
	return Vec2<T>(left.x * static_cast<T>(right), left.y * static_cast<T>(right));
}

template <typename T, typename U>
inline ff::Vec2<U> operator *(T left, const ff::Vec2<U>& right) {
	return Vec2<U>(right.x * left, right.y * left);
}

template <typename T, typename U>
inline ff::Vec2<T>& operator *=(ff::Vec2<T>& left, U right) {
	left.x *= right;
	left.y *= right;
	return left;
}

template <typename T, typename U>
inline ff::Vec2<T> operator /(const ff::Vec2<T>& left, U right) {
	if (right == 0) throw "Vec2: divide by zero";
	return Vec2<T>(left.x / static_cast<T>(right), left.y / static_cast<T>(right));
}

template <typename T, typename U>
inline ff::Vec2<T>& operator /=(ff::Vec2<T>& left, U right) {
	if (right == 0) throw "Vec2: divide by zero";
	left.x /= right;
	left.y /= right;
	return left;
}

template <typename T>
inline bool operator ==(const ff::Vec2<T>& left, const ff::Vec2<T>& right) {
	return (left.x == right.x) && (left.y == right.y);
}

template <typename T>
inline bool operator !=(const ff::Vec2<T>& left, const ff::Vec2<T>& right) {
	return (left.x != right.x) || (left.y != right.y);
}

template <typename T, typename>
inline bool operator <(const ff::Vec2<T>& left, const ff::Vec2<T>& right) {
	return left.y == right.y ?
		left.x < right.x :
		left.y < right.y;
}

template <typename T, typename>
inline bool operator <=(const ff::Vec2<T>& left, const ff::Vec2<T>& right) {
	return left == right || left < right;
}

template <typename T, typename>
inline bool operator >(const ff::Vec2<T>& left, const ff::Vec2<T>& right) {
	return !(left <= right);
}

template <typename T, typename>
inline bool operator >=(const ff::Vec2<T>& left, const ff::Vec2<T>& right) {
	return !(left < right);
}


//#include "Vec2.inl"
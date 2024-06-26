#pragma once

#include "fastfall/util/Vec2.hpp"
#include "fastfall/util/Line.hpp"
#include "fastfall/util/Angle.hpp"
#include "fastfall/util/Rect.hpp"
#include "fastfall/util/direction.hpp"

#include <algorithm>
#include <cmath>

namespace ff {

namespace math {

// General stuff
template <typename T>
[[nodiscard]]
inline T clamp(const T& v, const T& v1, const T& v2) {

	const T min = std::min(v1, v2);
	const T max = std::max(v1, v2);

	return std::min(std::max(min, v), max);
}

template <typename T>
[[nodiscard]]
inline T reduce(const T& val, const T& amount, const T& zero) {
	if (val > zero) {
		return std::max(val - amount, zero);
	}
	else if (val < zero) {
		return std::min(val + amount, zero);
	}
	return val;
}

template <typename T>
[[nodiscard]]
inline T increase(const T& val, const T& amount, const T& zero) {
	if (val > zero) {
		return val + amount;
	}
	else if (val < zero) {
		return val - amount;
	}
	return val;
}

// Vec2 stuff

template<typename T>
[[nodiscard]]
inline Vec2<T> lerp(const Vec2<T>& from, const Vec2<T>& to, std::floating_point auto lerp_val) {
	return Vec2<T>{
		std::lerp(from.x, to.x, lerp_val),
		std::lerp(from.y, to.y, lerp_val)
	};
}

template<typename T>
[[nodiscard]]
inline Vec2<T> unit(const Vec2<T>& a) {
	return a / a.magnitude();
}

[[nodiscard]]
inline Vec2f unit(const Angle& ang) {
    return { cosf(ang.radians()), sinf(ang.radians()) };
}

template<typename T>
[[nodiscard]]
inline Vec2<T> clamp(const Vec2<T>& vec, const Rect<T>& area) {
	return Vec2<T>(clamp(vec.x, area.left, area.left + area.width),
		clamp(vec.y, area.top, area.top + area.height));
}

template<typename T>
[[nodiscard]]
// < 0 means angle >90
// > 0 means angle <90
// ==0 means angle==90
inline T dot(const Vec2<T>& a, const Vec2<T>& b) {
	return (a.x * b.x) + (a.y * b.y);
}

template<typename T>
[[nodiscard]]
inline T cross(const Vec2<T>& a, const Vec2<T>& b) {
	return a.x * b.y - a.y * b.x;
}

template<typename T>
[[nodiscard]]
inline Vec2<T> cross(const Vec2<T>& a, T s) {
	return Vec2<T>(s * a.y, -s * a.x);
}

template<typename T>
[[nodiscard]]
inline Vec2<T> cross(T s, const Vec2<T>& a) {
	return Vec2<T>(-s * a.y, s * a.x);
}

template<typename T>
[[nodiscard]]
inline T distSquared(const Vec2<T>& a, const Vec2<T>& b) {
	return (a - b).magnitudeSquared();
}

template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
[[nodiscard]]
inline T dist(const Vec2<T>& a, const Vec2<T>& b) {
	return (a - b).magnitude();
}

template<typename T>
[[nodiscard]]
Vec2<T> normal(const Vec2<T>& a) {
	return Vec2<T>(a.y, -a.x);
}

template<typename T>
[[nodiscard]]
bool is_orthagonal(const Vec2<T>& a) {
	return a.x == 0.f || a.y == 0.f;
}

template<typename T>
[[nodiscard]]
bool is_horizontal(const Vec2<T>& a) {
	return a.y == 0.f;
}

template<typename T>
[[nodiscard]]
bool is_vertical(const Vec2<T>& a) {
	return a.x == 0.f;
}

template<typename T>
[[nodiscard]]
bool is_orthagonal(const Line<T>& a) {
	return a.p1.x == a.p2.x || a.p1.y == a.p2.y;
}

template<typename T>
[[nodiscard]]
bool is_horizontal(const Line<T>& a) {
	return a.p1.y == a.p2.y;
}

template<typename T>
[[nodiscard]]
bool is_vertical(const Line<T>& a) {
	return a.p1.x == a.p2.x;
}


template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
[[nodiscard]]
Vec2<T> rotate(const Vec2<T>& a, const Angle& ang) {
	return Vec2<T>{
		a.x * (T)cos(ang.radians()) - a.y * (T)sin(ang.radians()),
		a.x * (T)sin(ang.radians()) + a.y * (T)cos(ang.radians())
	};
}

// Line stuff
template<typename T>
[[nodiscard]]
inline T distSquared(const Line<T>& line) {
	return math::distSquared(line.p1, line.p2);
}

template<typename T>
[[nodiscard]]
inline T dist(const Line<T>& line) {
	return math::dist(line.p1, line.p2);
}

template<typename T>
[[nodiscard]]
Vec2<T> vector(const Line<T>& line) {
	return Vec2<T>(line.p2.x - line.p1.x, line.p2.y - line.p1.y);
}

template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
[[nodiscard]]
Vec2<T> projection(const Vec2<T>& a, const Vec2<T>& onto, bool ontoIsUnitVec = false) {
	if (onto.x == 0.f && onto.y == 0.f) {
		return Vec2<T>(0.f, 0.f);
	}
	else if (onto.x == 0.f) {
		return Vec2<T>(0.f, a.y);
	}
	else if (onto.y == 0.f) {
		return Vec2<T>(a.x, 0.f);
	}

	float dp = dot(a, onto);
	if (ontoIsUnitVec) {
		return Vec2<T>(dp * onto.x, dp * onto.y);
	}
	else {
		return Vec2<T>(
			(dp / (onto.x * onto.x + onto.y * onto.y)) * onto.x,
			(dp / (onto.x * onto.x + onto.y * onto.y)) * onto.y
			);
	}
}

template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
[[nodiscard]] 
constexpr Vec2<T> intersection(const Line<T>& a, const Line<T>& b) {

	double detA = (double)a.p1.x * (double)a.p2.y - (double)a.p1.y * (double)a.p2.x;
    double detB = (double)b.p1.x * (double)b.p2.y - (double)b.p1.y * (double)b.p2.x;

    double mxA = (double)a.p1.x - (double)a.p2.x;
    double mxB = (double)b.p1.x - (double)b.p2.x;

    double myA = (double)a.p1.y - (double)a.p2.y;
    double myB = (double)b.p1.y - (double)b.p2.y;

    double xnom = detA * mxB - detB * mxA;
    double ynom = detA * myB - detB * myA;

    double denom = mxA * myB - myA * mxB;

	if (denom == static_cast<T>(0)) {
		// is parallel
		return Vec2<T>(NAN, NAN);
	}

	double ixOut = xnom / denom;
    double iyOut = ynom / denom;

	if (!std::isfinite(ixOut) || !std::isfinite(iyOut)) {
		//numerical issue?
		return Vec2<T>(NAN, NAN);
	}

	return Vec2<T>(static_cast<T>(ixOut), static_cast<T>(iyOut));
}


template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
[[nodiscard]]
constexpr bool collinear(const Line<T>& a, const Line<T>& b) {

    double x1 = a.p1.x;
    double x2 = a.p2.x;
    double x3 = b.p2.x;

    double y1 = a.p1.y;
    double y2 = a.p2.y;
    double y3 = b.p2.y;

    double d = x1 * (y2 - y3)
             + x2 * (y3 - y1)
             + x3 * (y1 - y2);

    return d == 0.f;
}


template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
[[nodiscard]] 
constexpr Vec2<T> midpoint(const Line<T>& a) {
	return a.p1 + (math::vector(a) / static_cast<T>(2.0));
}

template<typename T>
[[nodiscard]] 
constexpr Line<T> shift(const Line<T>& a, const Vec2<T>& offset) {
	Line<T> b = a;
	b.p1 += offset;
	b.p2 += offset;
	return b;
}

template<typename T>
[[nodiscard]]
constexpr Vec2<T> normal(const Line<T>& a) {
	return math::vector(a).unit().lefthand();
}

template<typename T>
[[nodiscard]]
constexpr Vec2<T> tangent(const Line<T>& a) {
	return math::vector(a).unit();
}

// Rect stuff

template<typename T>
[[nodiscard]] 
constexpr Rect<T> shift(const Rect<T>& a, const Vec2<T>& offset) {
	Rect<T> b = a;
	b.left += offset.x;
	b.top  += offset.y;
	return b;
}

template<typename T>
[[nodiscard]] constexpr Vec2<T> rect_topleft (const Rect<T>& a) { return Vec2<T>(a.left,                  a.top); }
template<typename T>
[[nodiscard]] constexpr Vec2<T> rect_topmid  (const Rect<T>& a) { return Vec2<T>(a.left + a.width / T{2}, a.top); }
template<typename T>
[[nodiscard]] constexpr Vec2<T> rect_topright(const Rect<T>& a) { return Vec2<T>(a.left + a.width,        a.top); }
template<typename T>
[[nodiscard]] constexpr Vec2<T> rect_leftmid (const Rect<T>& a) { return Vec2<T>(a.left,                  a.top + a.height / T{2}); }
template<typename T>
[[nodiscard]] constexpr Vec2<T> rect_center  (const Rect<T>& a) { return Vec2<T>(a.left + a.width / T{2}, a.top + a.height / T{2}); }
template<typename T>
[[nodiscard]] constexpr Vec2<T> rect_rightmid(const Rect<T>& a) { return Vec2<T>(a.left + a.width,        a.top + a.height / T{2}); }
template<typename T>
[[nodiscard]] constexpr Vec2<T> rect_botright(const Rect<T>& a) { return Vec2<T>(a.left + a.width,        a.top + a.height); }
template<typename T>
[[nodiscard]] constexpr Vec2<T> rect_botmid  (const Rect<T>& a) { return Vec2<T>(a.left + a.width / T{2}, a.top + a.height); }
template<typename T>
[[nodiscard]] constexpr Vec2<T> rect_botleft (const Rect<T>& a) { return Vec2<T>(a.left,                  a.top + a.height); }

template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
[[nodiscard]]
constexpr Vec2<T> rect_mid(const Rect<T>& a) {
	return Vec2<T>(a.left + (a.width / 2.f), a.top + (a.height / 2.f));
}

template<typename T>
[[nodiscard]]
Rect<T> rect_bound(const Rect<T>& a, const Rect<T>& b) {
	Rect<T> r;
	r.left = std::min(a.left, b.left);
	r.top = std::min(a.top, b.top);
	r.width = std::max(a.left + a.width, b.left + b.width) - r.left;
	r.height = std::max(a.top + a.height, b.top + b.height) - r.top;
	return r;
}

template<typename T>
[[nodiscard]]
Rect<T> rect_extend(const Rect<T>& a, Cardinal dir, T amount) {
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

// Angle stuff
template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
[[nodiscard]]
Angle angle(const Vec2<T>& a) {
	return Angle(std::atan2(a.y, a.x));
}

template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
[[nodiscard]]
Angle angle(const Line<T>& a) {
	return Angle(std::atan2(a.p2.y - a.p1.y, a.p2.x - a.p1.x));
}

// Line stuff
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
bool line_has_point(const Line<T> &a, const Vec2<T>& p, float tolerance = 0.f) {
    float a_dist = math::dist(a.p1, a.p2);
    float b_dist = math::dist(a.p1, p) + math::dist(p, a.p2);

    return b_dist <= a_dist + tolerance
       &&  b_dist >= a_dist - tolerance;
}

}

}

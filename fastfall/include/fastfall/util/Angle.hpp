#pragma once

#include <cmath>
#include <string>
#include <numbers>

namespace ff {

class Angle {
public:

    constexpr static inline float PI = std::numbers::pi_v<float>;

	constexpr Angle() {
		rad = 0.f;
		deg = 0.f;
	}
	constexpr Angle(float radians) {
		setRad(radians);
	};
	constexpr Angle(const Angle& a) {
		rad = a.radians();
		deg = a.degrees();
	}

	constexpr void setRad(float radians) {
		rad = radians;
		calcDeg();
		normalize();
	}
	constexpr void setDeg(float degrees) {
		deg = degrees;
		calcRad();
		normalize();
	}

	constexpr inline float degrees() const {
		return deg;
	}
	constexpr inline float radians() const {
		return rad;
	}

	constexpr inline static Angle Degree(float degree) {
		return Angle(Angle::toRadian(degree));
	}
	constexpr inline static Angle Radian(float radian) {
		return Angle(radian);
	}

	constexpr inline static float toDegree(float radian) {
		return radian * 180.f / PI;
	}
	constexpr inline static float toRadian(float degree) {
		return degree * PI / 180.f;
	}

	std::string to_string() const noexcept {
		return std::to_string(deg) + "deg";
	}


    auto operator<=>(const Angle&) const = default;

protected:
	float rad;
	float deg;

	constexpr inline void calcDeg() {
		deg = rad * 180.f / PI;
	}
	constexpr inline void calcRad() {
		rad = deg * PI / 180.f;
	}
	constexpr void normalize() {

		//normal range is
		// -180.f < deg <= 180.f
		// -PI_F  < rad <= PI_F

		if (deg > 180.f || deg <= -180.f) {
			deg = remainder(deg + 180.f, 360.f) - 180.f;
			rad = remainder(rad + PI, 2.f * PI) - PI;

			if (deg <= -180.f) {
				deg += 360.f;
				rad += 2.f * PI;
			}
			else if (deg > 180.f) {
				deg -= 360.f;
				rad -= 2.f * PI;
			}
		}
	}

	// dunno why this doesn't already exist
	static constexpr float constexpr_abs(float v)
	{
		return (v < 0.f ? -v : v);
	}

	static constexpr float remainder(float _X, float _Y) {
		float sign = (_X < 0.f ? -1.f : 1.f);
		return sign * (constexpr_abs(_X) - static_cast<long long int>(constexpr_abs(_X / _Y)) * constexpr_abs(_Y));
	}

	friend constexpr bool operator< (const Angle& lhs, const Angle& rhs) noexcept {
		return lhs.radians() < rhs.radians();
	}
	friend constexpr bool operator<= (const Angle& lhs, const Angle& rhs) noexcept {
		return lhs.radians() <= rhs.radians();
	}

	friend constexpr bool operator> (const Angle& lhs, const Angle& rhs) noexcept {
		return lhs.radians() > rhs.radians();
	}
	friend constexpr bool operator>= (const Angle& lhs, const Angle& rhs) noexcept {
		return lhs.radians() >= rhs.radians();
	}

	friend constexpr bool operator== (const Angle& lhs, const Angle& rhs) noexcept {
		return lhs.radians() == rhs.radians();
	}
	friend constexpr bool operator!= (const Angle& lhs, const Angle& rhs) noexcept {
		return lhs.radians() != rhs.radians();
	}

	friend constexpr Angle operator -(const Angle& right) {
		return Angle(-right.radians());
	}
	friend constexpr Angle& operator +=(Angle& left, const Angle& right) {
		left.setRad(left.radians() + right.radians());
		return left;
	}
	friend constexpr Angle& operator -=(Angle& left, const Angle& right) {
		left.setRad(left.radians() - right.radians());
		return left;
	}

	friend constexpr Angle operator +(const Angle& left, const Angle& right) {
		return Angle(left.radians() + right.radians());
	}
	friend constexpr Angle operator -(const Angle& left, const Angle& right) {
		return Angle(left.radians() - right.radians());
	}
	friend constexpr Angle operator *(const Angle& left, float right) {
		return Angle(left.radians() * right);
	}
	friend constexpr Angle operator *(float left, const Angle& right) {
		return Angle(right.radians() * left);
	}
	friend constexpr Angle& operator *=(Angle& left, float right) {
		left.setRad(left.radians() * right);
		return left;
	}
	friend constexpr Angle operator /(const Angle& left, float right) {
		return Angle(left.radians() / right);
	}
	friend constexpr Angle& operator /=(Angle& left, float right) {
		left.setRad(left.radians() / right);
		return left;
	}

	friend constexpr Angle operator /(const Angle& left, Angle right) {
		return Angle(left.radians() / right.radians());
	}
	friend constexpr Angle& operator /=(Angle& left, Angle right) {
		left.setRad(left.radians() / right.radians());
		return left;
	}

	friend constexpr Angle abs(const Angle& ang) {
		return (ang >= Angle{} ? ang : -ang);
	}
};

struct AngleRange {
    Angle min;
    Angle max;
    bool  inclusive = true;

    [[nodiscard]]
    bool contains(Angle ang) const {
        if (inclusive) {
            return ang.radians() >= min.radians() && ang.radians() <= max.radians();
        }
        else {
            return ang.radians() > min.radians() && ang.radians() < max.radians();
        }
    };

    const static AngleRange Any;
};

inline const AngleRange AngleRange::Any = {
    .min = Angle::Degree(std::nextafterf(-180.f, 0.f)),
    .max = Angle::Degree(180.f),
    .inclusive = true
};

}


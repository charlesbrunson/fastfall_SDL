#pragma once

#include <cmath>
#include <string>

namespace ff {

constexpr float PI_F = 3.14159265358979323846264f;

class Angle {
public:

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

	bool isBetween(const Angle& angStart, const Angle& angEnd, bool inclusive = true) const {
		if (inclusive) {
			return rad >= angStart.rad && rad <= angEnd.rad;
		}
		else {
			return rad > angStart.rad && rad < angEnd.rad;
		}
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
		return radian * 180.f / PI_F;
	}
	constexpr inline static float toRadian(float degree) {
		return degree * PI_F / 180.f;
	}

	std::string to_string() const noexcept {
		return std::to_string(deg) + "deg";
	}


	bool operator< (const Angle& rhs) {
		return rad < rhs.rad;
	}
	bool operator== (const Angle& rhs) {
		return rad == rhs.rad;
	}
	bool operator!= (const Angle& rhs) {
		return rad != rhs.rad;
	}
	bool operator> (const Angle& rhs) {
		return rad > rhs.rad;
	}

protected:
	float rad;
	float deg;

	constexpr void calcDeg() {
		deg = rad * 180.f / PI_F;
	}
	constexpr void calcRad() {
		rad = deg * PI_F / 180.f;
	}
	constexpr void normalize() {

		//normal range is
		// -180.f < deg <= 180.f
		// -PI_F  < rad <= PI_F

		if (deg > 180.f || deg <= -180.f) {
			deg = remainder(deg + 180.f, 360.f) - 180.f;
			rad = remainder(rad + PI_F, 2.f * PI_F) - PI_F;

			if (deg <= -180.f) {
				deg += 360.f;
				rad += 2.f * PI_F;
			}
			else if (deg > 180.f) {
				deg -= 360.f;
				rad -= 2.f * PI_F;
			}
		}
	}

	static constexpr float remainder(float _X, float _Y) {

		// no idea if this works
		return (_X < float() ? float(-1) : float(1)) * (
			(_X < float() ? -_X : _X) -
			static_cast<long long int>((_X / _Y < float() ? -_X / _Y : _X / _Y)) * (_Y < float() ? -_Y : _Y)
			);
	}
};
inline constexpr Angle operator -(const Angle& right) {
	return Angle(-right.radians());
}
inline constexpr Angle& operator +=(Angle& left, const Angle& right) {
	left.setRad(left.radians() + right.radians());
	return left;
}
inline constexpr Angle& operator -=(Angle& left, const Angle& right) {
	left.setRad(left.radians() - right.radians());
	return left;
}
inline constexpr Angle operator +(const Angle& left, const Angle& right) {
	return Angle(left.radians() + right.radians());
}
inline constexpr Angle operator -(const Angle& left, const Angle& right) {
	return Angle(left.radians() - right.radians());
}
inline constexpr Angle operator *(const Angle& left, float right) {
	return Angle(left.radians() * right);
}
inline constexpr Angle operator *(float left, const Angle& right) {
	return Angle(right.radians() * left);
}
inline constexpr Angle& operator *=(Angle& left, float right) {
	left.setRad(left.radians() * right);
	return left;
}
inline constexpr Angle operator /(const Angle& left, float right) {
	return Angle(left.radians() / right);
}
inline constexpr Angle& operator /=(Angle& left, float right) {
	left.setRad(left.radians() / right);
	return left;
}
inline constexpr bool operator ==(const Angle& left, const Angle& right) {
	return left.radians() == right.radians();
}
inline constexpr bool operator !=(const Angle& left, const Angle& right) {
	return left.radians() != right.radians();
}

//std::ostream& operator<<(std::ostream& os, const Angle& dt);

//#include "Angle.inl"

}
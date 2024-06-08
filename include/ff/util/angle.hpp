#pragma once

#include <cmath>
#include <string>
#include <numbers>

namespace ff {

class angle {
public:
    constexpr static inline float PI = std::numbers::pi_v<float>;

    constexpr angle() {
        rad = 0.f;
        deg = 0.f;
    }
    constexpr angle(float radians) {
        rad = radians;
        calcDeg();
        normalize();
    };
    constexpr angle(const angle& a) {
        rad = a.radians();
        deg = a.degrees();
    }

    constexpr inline float degrees() const {
        return deg;
    }
    constexpr inline float radians() const {
        return rad;
    }

    constexpr inline static angle from_degrees(float degree) {
        return angle(angle::radians(degree));
    }
    constexpr inline static angle from_radians(float radian) {
        return angle(radian);
    }

    constexpr inline static float degrees(float radian) {
        return radian * 180.f / PI;
    }
    constexpr inline static float radians(float degree) {
        return degree * PI / 180.f;
    }

    std::string to_string() const noexcept {
        return std::to_string(deg) + "deg";
    }

    auto operator<=>(const angle&) const = default;

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
    static constexpr inline float constexpr_abs(float v)
    {
        return (v < 0.f ? -v : v);
    }

    static constexpr inline float remainder(float _X, float _Y) {
        float sign = (_X < 0.f ? -1.f : 1.f);
        return sign * (constexpr_abs(_X) - static_cast<long long int>(constexpr_abs(_X / _Y)) * constexpr_abs(_Y));
    }
};

struct angle_range {
    angle min;
    angle max;
    bool  inclusive = true;

    [[nodiscard]]
    inline bool contains(angle ang) const {
        if (inclusive) {
            return ang.radians() >= min.radians() && ang.radians() <= max.radians();
        }
        else {
            return ang.radians() > min.radians() && ang.radians() < max.radians();
        }
    };

    const static angle_range any;
};

inline const angle_range angle_range::any = {
    .min = angle::from_degrees(std::nextafterf(-180.f, 0.f)),
    .max = angle::from_degrees(180.f),
    .inclusive = true
};

}

constexpr bool operator< (const ff::angle& lhs, const ff::angle& rhs) noexcept {
    return lhs.radians() < rhs.radians();
}
constexpr bool operator<= (const ff::angle& lhs, const ff::angle& rhs) noexcept {
    return lhs.radians() <= rhs.radians();
}

constexpr bool operator> (const ff::angle& lhs, const ff::angle& rhs) noexcept {
    return lhs.radians() > rhs.radians();
}
constexpr bool operator>= (const ff::angle& lhs, const ff::angle& rhs) noexcept {
    return lhs.radians() >= rhs.radians();
}

constexpr bool operator== (const ff::angle& lhs, const ff::angle& rhs) noexcept {
    return lhs.radians() == rhs.radians();
}
constexpr bool operator!= (const ff::angle& lhs, const ff::angle& rhs) noexcept {
    return lhs.radians() != rhs.radians();
}

inline constexpr ff::angle operator -(const ff::angle& right) {
    return ff::angle(-right.radians());
}
inline constexpr ff::angle& operator +=(ff::angle& left, const ff::angle& right) {
    left = ff::angle(left.radians() + right.radians());
    return left;
}
inline constexpr ff::angle& operator -=(ff::angle& left, const ff::angle& right) {
    left = ff::angle(left.radians() - right.radians());
    return left;
}

inline constexpr ff::angle operator +(const ff::angle& left, const ff::angle& right) {
    return ff::angle(left.radians() + right.radians());
}
inline constexpr ff::angle operator -(const ff::angle& left, const ff::angle& right) {
    return ff::angle(left.radians() - right.radians());
}
inline constexpr ff::angle operator *(const ff::angle& left, float right) {
    return ff::angle(left.radians() * right);
}
inline constexpr ff::angle operator *(float left, const ff::angle& right) {
    return ff::angle(right.radians() * left);
}
inline constexpr ff::angle& operator *=(ff::angle& left, float right) {
    left = ff::angle(left.radians() * right);
    return left;
}
inline constexpr ff::angle operator /(const ff::angle& left, float right) {
    return ff::angle(left.radians() / right);
}
inline constexpr ff::angle& operator /=(ff::angle& left, float right) {
    left = ff::angle(left.radians() / right);
    return left;
}

inline constexpr ff::angle operator /(const ff::angle& left, ff::angle right) {
    return ff::angle(left.radians() / right.radians());
}
inline constexpr ff::angle& operator /=(ff::angle& left, ff::angle right) {
    left = ff::angle(left.radians() / right.radians());
    return left;
}

inline constexpr ff::angle abs(const ff::angle& ang) {
    return (ang >= ff::angle{} ? ang : -ang);
}

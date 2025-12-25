#pragma once

#include "fastfall/util/Rect.hpp"

#include <iostream>

namespace ff
{
    struct fixed32_8
    {
        constexpr static unsigned int SIGN_MASK = 0x80000000;

        constexpr static unsigned int INTEGER_BITS = 23;
        constexpr static unsigned int INTEGER_STEPS = 1 << INTEGER_BITS;
        constexpr static unsigned int INTEGER_MASK = 0x7FFFFF00;

        constexpr static unsigned int FRACTION_BITS = 8;
        constexpr static unsigned int FRACTION_STEPS = 1 << FRACTION_BITS;
        constexpr static unsigned int FRACTION_MASK = 0x000000FF;

        int32_t underlying = 0;

        constexpr fixed32_8& operator=(int rhs)
        {
            auto tmp = create(rhs);
            *this = tmp;
            return *this;
        }

        constexpr fixed32_8& operator=(float rhs)
        {
            auto tmp = from_float(rhs);
            *this = tmp;
            return *this;
        }

        constexpr static fixed32_8 create(int32_t integer, int16_t fraction = 0)
        {
            fixed32_8 num;

            if (fraction < 0)
            {
                integer--;
                fraction = FRACTION_STEPS + fraction;
            }

            num.underlying = ((static_cast<uint32_t>(integer < 0) << 31) & SIGN_MASK)
                | ((integer << FRACTION_BITS) & INTEGER_MASK)
                | (fraction & FRACTION_MASK);

            return num;
        }

        constexpr static fixed32_8 from_raw(int32_t value)
        {
            fixed32_8 num;
            num.underlying = value;
            return num;
        }

        constexpr static fixed32_8 from_float(long double value)
        {
            fixed32_8 num;
            num.underlying = static_cast<int32_t>(value * FRACTION_STEPS);
            return num;
        }

        fixed32_8 operator+ (const fixed32_8& rhs) const
        {
           return from_raw(underlying + rhs.underlying);
        }

        fixed32_8 operator- (const fixed32_8& rhs) const
        {
            return from_raw(underlying - rhs.underlying);
        }

        fixed32_8 operator* (const fixed32_8& rhs) const
        {
            int64_t result = static_cast<int64_t>(underlying) * static_cast<int64_t>(rhs.underlying);
            return from_raw(result >> FRACTION_BITS);
        }

        fixed32_8 operator/ (const fixed32_8& rhs) const
        {
            int64_t result = (static_cast<int64_t>(underlying) << FRACTION_BITS) / static_cast<int64_t>(rhs.underlying);
            return from_raw(static_cast<int32_t>(result));
        }

        fixed32_8 operator-() const
        {
            return fixed32_8{} - *this;
        }

        fixed32_8& operator+= (const fixed32_8& rhs)
        {
            underlying = (*this + rhs).underlying;
            return *this;
        }

        fixed32_8& operator-= (const fixed32_8& rhs)
        {
            underlying = (*this - rhs).underlying;
            return *this;
        }

        fixed32_8& operator*= (const fixed32_8& rhs)
        {
            underlying = (*this * rhs).underlying;
            return *this;
        }

        fixed32_8& operator/= (const fixed32_8& rhs)
        {
            underlying = (*this / rhs).underlying;
            return *this;
        }

        explicit constexpr operator float() const
        {
            return static_cast<float>(underlying) / static_cast<float>(FRACTION_STEPS);
        }

        explicit constexpr operator double() const
        {
            return static_cast<double>(underlying) / static_cast<double>(FRACTION_STEPS);
        }

        friend constexpr std::strong_ordering operator<=> (const fixed32_8& lhs, const fixed32_8& rhs)
        {
            if (lhs.underlying < rhs.underlying)
                return std::strong_ordering::less;
            if (lhs.underlying > rhs.underlying)
                return std::strong_ordering::greater;
            return std::strong_ordering::equal;
        }

        friend constexpr bool operator== (const fixed32_8& lhs, const fixed32_8& rhs)
        {
            return lhs.underlying == rhs.underlying;
        }

        friend constexpr bool operator!= (const fixed32_8& lhs, const fixed32_8& rhs)
        {
            return lhs.underlying != rhs.underlying;
        }

        friend constexpr bool operator< (const fixed32_8& lhs, const fixed32_8& rhs)
        {
            return lhs.underlying < rhs.underlying;
        }
        friend constexpr bool operator<= (const fixed32_8& lhs, const fixed32_8& rhs)
        {
            return lhs.underlying <= rhs.underlying;
        }
        friend constexpr bool operator> (const fixed32_8& lhs, const fixed32_8& rhs)
        {
            return lhs.underlying > rhs.underlying;
        }
        friend constexpr bool operator>= (const fixed32_8& lhs, const fixed32_8& rhs)
        {
            return lhs.underlying >= rhs.underlying;
        }

        friend std::ostream& operator<<(std::ostream& os, const fixed32_8& value)
        {
            return os << static_cast<double>(value);
        }


    };

    using Vec2fx = Vec2<fixed32_8>;
    using Linefx = Line<fixed32_8>;
    using Rectfx = Rect<fixed32_8>;

    constexpr fixed32_8 operator""_fx(long double value) {
        fixed32_8 result;
        result.underlying = static_cast<int32_t>(value * fixed32_8::FRACTION_STEPS);
        return result;
    }

    constexpr fixed32_8 operator""_fx(unsigned long long int value) {
        fixed32_8 result;
        result.underlying = static_cast<int32_t>(value * fixed32_8::FRACTION_STEPS);
        return result;
    }

    constexpr double format_as(const fixed32_8 value)
    {
        return static_cast<double>(value);
    }

}

#pragma once

#include <optional>
#include <concepts>
#include <array>
#include <fmt/format.h>
#include <glm/vec2.hpp>

#include "angle.hpp"

namespace ff {
enum class Cardinal : uint8_t { N, E, S, W };
enum class Ordinal : uint8_t { NW, NE, SE, SW };

template<typename T, typename DirectionEnum>
struct directional_array
{
    using Direction = DirectionEnum;

    constexpr T& operator[] (Direction dir)
    {
        return data[static_cast<size_t>(dir)];
    }
    constexpr const T& operator[] (Direction dir) const
    {
        return data[static_cast<size_t>(dir)];
    }

    using iterator = typename std::array<T, 4>::iterator;
    using const_iterator = typename std::array<T, 4>::const_iterator;
    using reverse_iterator = typename std::array<T, 4>::reverse_iterator;
    using const_reverse_iterator = typename std::array<T, 4>::const_reverse_iterator;

    constexpr iterator					begin()			  { return data.begin();	};
    constexpr iterator					end()			  { return data.end();		};
    constexpr const_iterator			begin()		const { return data.begin();	};
    constexpr const_iterator			end()		const { return data.end();		};
    constexpr const_iterator			cbegin()	const { return data.cbegin();	};
    constexpr const_iterator			cend()		const { return data.cend();		};
    constexpr reverse_iterator			rbegin()		  { return data.rbegin();	};
    constexpr reverse_iterator			rend()		      { return data.rend();		};
    constexpr const_reverse_iterator	crbegin()	const { return data.crbegin();	};
    constexpr const_reverse_iterator	crend()		const { return data.crend();	};

    std::array<T, 4> data;
};

template<typename T>
using cardinal_array = directional_array<T, Cardinal>;

template<typename T>
using ordinal_array = directional_array<T, Ordinal>;

namespace direction {

inline constexpr auto cardinals = { Cardinal::N, Cardinal::E, Cardinal::S, Cardinal::W };
inline constexpr auto ordinals = { Ordinal::NW, Ordinal::NE, Ordinal::SE, Ordinal::SW };

inline Cardinal opposite(Cardinal dir)
{
    return static_cast<Cardinal>((static_cast<unsigned>(dir) + 2) % 4);
}
inline Ordinal opposite(Ordinal dir)
{
    return static_cast<Ordinal>((static_cast<unsigned>(dir) + 2) % 4);
}

inline std::optional<Ordinal> combine(Cardinal lhs, Cardinal rhs)
{
    if (lhs > rhs)
        std::swap(lhs, rhs);

    if (lhs == Cardinal::N && rhs == Cardinal::E)
        return Ordinal::NE;
    if (lhs == Cardinal::N && rhs == Cardinal::W)
        return Ordinal::NW;
    if (lhs == Cardinal::E && rhs == Cardinal::S)
        return Ordinal::SE;
    if (lhs == Cardinal::S && rhs == Cardinal::W)
        return Ordinal::SW;

    return {};
}

inline std::pair<Cardinal, Cardinal> split(Ordinal ord)
{
    std::pair<Cardinal, Cardinal> pair;
    switch (ord)
    {
        case Ordinal::NW: pair = { Cardinal::N, Cardinal::W }; break;
        case Ordinal::NE: pair = { Cardinal::N, Cardinal::E }; break;
        case Ordinal::SE: pair = { Cardinal::S, Cardinal::E }; break;
        case Ordinal::SW: pair = { Cardinal::S, Cardinal::W }; break;
    }
    return pair;
}

inline std::pair<Ordinal, Ordinal> split(Cardinal ord)
{
    std::pair<Ordinal, Ordinal> pair;
    switch (ord)
    {
        case Cardinal::N: pair = { Ordinal::NW, Ordinal::NE }; break;
        case Cardinal::E: pair = { Ordinal::NE, Ordinal::SE }; break;
        case Cardinal::S: pair = { Ordinal::SE, Ordinal::SW }; break;
        case Cardinal::W: pair = { Ordinal::SW, Ordinal::NW }; break;
    }
    return pair;
}

template<typename T = int>
requires (std::signed_integral<T> || std::floating_point<T>)
inline glm::vec<2, T> to_vector(Cardinal card)
{
    glm::vec<2, T> v;
    switch (card)
    {
        case Cardinal::N: v = glm::vec<2, T>{ 0, -1 }; break;
        case Cardinal::E: v = glm::vec<2, T>{ 1,  0 }; break;
        case Cardinal::S: v = glm::vec<2, T>{ 0,  1 }; break;
        case Cardinal::W: v = glm::vec<2, T>{-1,  0 }; break;
    }
    return v;
}

template<typename T = int>
requires (std::signed_integral<T> || std::floating_point<T>)
inline glm::vec<2, T> to_vector(Ordinal card)
{
    glm::vec<2, T> v;
    switch (card)
    {
        case Ordinal::NW: v = glm::vec<2, T>{-1, -1 }; break;
        case Ordinal::NE: v = glm::vec<2, T>{ 1, -1 }; break;
        case Ordinal::SE: v = glm::vec<2, T>{ 1,  1 }; break;
        case Ordinal::SW: v = glm::vec<2, T>{-1,  1 }; break;
    }
    return v;
}

inline angle to_angle(Cardinal card)
{
    angle ang;
    switch (card)
    {
        case Cardinal::N: ang = angle{ -90.f };
        case Cardinal::E: ang = angle{ 0.f };
        case Cardinal::S: ang = angle{ 90.f };
        case Cardinal::W: ang = angle{ 180.f };
    }
    return ang;
}

template<class ... Directions>
requires (std::same_as<Directions, Cardinal> && ...)
constexpr unsigned to_bits(Directions... dir) noexcept {
    return ( (1u << static_cast<unsigned>(dir)) | ...);
};

template<class ... Directions>
requires (std::same_as<Directions, Ordinal> && ...)
constexpr unsigned to_bits(Directions... dir) noexcept {
    return ((1u << static_cast<unsigned>(dir)) | ...);
};

template<typename T>
requires (std::signed_integral<T> || std::floating_point<T>)
std::optional<Cardinal> from_vector(const glm::vec<2, T>& v)
{
    if (v.x == 0)
    {
        if (v.y < 0)
            return Cardinal::N;
        else if (v.y > 0)
            return Cardinal::S;
    }
    else if (v.y == 0)
    {
        if (v.x < 0)
            return Cardinal::W;
        else if (v.x > 0)
            return Cardinal::E;
    }
    return {};
}

inline std::optional<Cardinal> cardinal_from_str(std::string_view str) {
    if (!str.empty()) {
        switch(str.at(0)) {
            case 'n':
            case 'N':
                return Cardinal::N;
            case 'e':
            case 'E':
                return Cardinal::E;
            case 's':
            case 'S':
                return Cardinal::S;
            case 'w':
            case 'W':
                return Cardinal::W;
        }
    }
    return {};
}

}

}

template <> struct fmt::formatter<ff::Cardinal> : formatter<string_view> {
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(ff::Cardinal dir, FormatContext& ctx) {
        string_view name = "unknown";
        switch (dir) {
            case ff::Cardinal::N: name = "N"; break;
            case ff::Cardinal::E: name = "E"; break;
            case ff::Cardinal::S: name = "S"; break;
            case ff::Cardinal::W: name = "W"; break;
        }
        return formatter<string_view>::format(name, ctx);
    }
};

template <> struct fmt::formatter<ff::Ordinal> : formatter<string_view> {
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(ff::Ordinal dir, FormatContext& ctx) {
        string_view name = "unknown";
        switch (dir) {
            case ff::Ordinal::NW: name = "NW"; break;
            case ff::Ordinal::NE: name = "NE"; break;
            case ff::Ordinal::SE: name = "SE"; break;
            case ff::Ordinal::SW: name = "SW"; break;
        }
        return formatter<string_view>::format(name, ctx);
    }
};

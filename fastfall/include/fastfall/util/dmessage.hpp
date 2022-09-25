#pragma once

#include <array>
#include <cinttypes>
#include <string>
#include <variant>

#include "math.hpp"

namespace ff {
namespace detail {
    constexpr unsigned int hash(const char *str) {
        constexpr unsigned MULTIPLIER = 37u;
        unsigned int hash = 0u;
        for (auto *p = str; *p != '\0'; p++)
            hash = MULTIPLIER * hash + (unsigned char)*p;
        return hash;
    }
}

template<size_t N>
struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }

    char value[N];

    constexpr const char* data() const { return value; }
    constexpr operator std::string_view() const {
        return std::string_view{data()};
    }

    constexpr size_t size() const { return sizeof(value); }
};

// 32 bit string hash
//constexpr unsigned int hash(char *str);

enum class dtype : uint8_t {
    Nil     = 0,
    Bool    = 1,
    Int     = 2,
    Float   = 3,
    Vec2i   = 4,
    Vec2f   = 5,
    Spare1  = 6,
    Spare2  = 7,
    // can go to 15 (4 bits) actually
};

using dparam = std::variant<
        std::monostate,
        bool,
        int,
        float,
        Vec2i,
        Vec2f
    >;

// dtype enum to datatype helpers
template<dtype T> struct to_type {};
template<> struct to_type<dtype::Nil>      { using type = std::monostate; };
template<> struct to_type<dtype::Bool>     { using type = bool; };
template<> struct to_type<dtype::Int>      { using type = int; };
template<> struct to_type<dtype::Float>    { using type = float; };
template<> struct to_type<dtype::Vec2i>    { using type = Vec2i; };
template<> struct to_type<dtype::Vec2f>    { using type = Vec2f; };
template<> struct to_type<dtype::Spare1>   { using type = void; };
template<> struct to_type<dtype::Spare2>   { using type = void; };

template<dtype T>
using to_type_t = typename to_type<T>::type;

class dmessage {
public:

    constexpr dmessage
    (
        uint64_t t_hash,
        std::array<dparam, 4> t_params,
        unsigned t_count
    )
        : type_hash(t_hash)
        , param_data(t_params)
        , param_count(t_count)
    {
    }

    constexpr operator size_t() const { return type_hash; }
    constexpr uint64_t hash() const { return type_hash; }

    const std::array<dparam, 4>& params() const { return param_data; }
    unsigned pcount() const { return param_count; }

private:
    uint64_t type_hash;
    std::array<dparam, 4> param_data;
    unsigned param_count = 0;
};

template<StringLiteral Name, dtype RType, dtype... Params>
    requires (sizeof...(Params) <= 4)
class dformat {
private:
    std::array<dtype, 4> param_types;
    unsigned param_count = 0;
    dtype rtype;

    uint64_t type_hash;

    constexpr static std::string_view type_name = Name;

public:
    constexpr dformat()
        : rtype(RType)
        , param_types(std::array<dtype, 4>{ Params... })
        , param_count(sizeof...(Params))
    {
        unsigned name_hash = detail::hash(Name.data());
        unsigned param_hash = 0;

        unsigned i = 0;
        for (auto& type : param_types) {
            param_hash |= (static_cast<uint8_t>(type) & 0b1111) << (4 * i);
            ++i;
        }
        param_hash |= (static_cast<uint8_t>(rtype) & 0b1111) << (4 * i);
        type_hash = ((uint64_t)name_hash << 32lu) | (uint64_t)param_hash;
    }

    constexpr operator uint64_t() const { return type_hash; }

    // unwrap message parameters
    constexpr
    std::tuple<to_type_t<Params>...>
    unwrap(const dmessage& msg) const
    {
        if (msg.hash() == type_hash) {
            // what fresh hell is this
            return [&]<uint64_t...Is>(std::index_sequence<Is...>) {
                return std::make_tuple(
                        std::get<to_type_t<Params>>(msg.params()[Is])...);
            }(std::make_index_sequence<sizeof...(Params)>{});
        }
        else {
            throw std::exception{};
        }
    }

    // wraps parameters into a message
    constexpr
    dmessage
    wrap(to_type_t<Params>... params) const
    {
        return { type_hash, { params... }, sizeof...(Params)};
    }

    // wraps message return param
    constexpr
    dparam
    wrap_r(to_type_t<RType> r) const {
        return { r };
    }

    // unwrap message return param
    constexpr
    to_type_t<RType>
    unwrap_r(std::optional<dparam> r) const {
        if constexpr (RType != dtype::Nil) {
            if (r && r->index() == dparam{to_type_t<RType>{}}.index()) {
                return std::get<to_type_t<RType>>(*r);
            }
            else {
                throw std::exception{};
            }
        }
        else {
            if (!r) {
                return {};
            }
            else {
                throw std::exception{};
            }
        }
    }

    template<class Sendable>
    constexpr std::conditional_t<RType == dtype::Nil, void, to_type_t<RType>>
    send(Sendable& mailbox, to_type_t<Params>... params) const {
        if constexpr (RType == dtype::Nil) {
            unwrap_r(mailbox.message(wrap(std::forward<to_type_t<Params>>(params)...)));
        }
        else {
            return unwrap_r(mailbox.message(wrap(std::forward<to_type_t<Params>>(params)...)));
        }
    }
};


}
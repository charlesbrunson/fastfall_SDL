#pragma once

#include <array>
#include <cinttypes>
#include <string>
#include <variant>

#include "math.hpp"

namespace ff {

template<size_t N>
struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }

    char value[N];

    const char* data() const { return value; }
    operator std::string_view() const {
        return std::string_view{data()};
    }

    constexpr size_t size() const { return sizeof(value); }
};

// 32 bit string hash
constexpr unsigned int hash(char *str)
{
    constexpr unsigned MULTIPLIER = 37u;
    unsigned int hash = 0u;
    for (unsigned char* p = (unsigned char*)str; *p != '\0'; p++)
        hash = MULTIPLIER * hash + *p;
    return hash;
}

enum class dtype : uint8_t {
    Nil     = 0,
    Bool    = 1,
    Int     = 2,
    Float   = 3,
    Vec2i   = 4,
    Vec2f   = 5,
    Spare1  = 6,
    Spare2  = 7,
};

using dparam = std::variant<
        std::monostate,
        bool,
        int,
        float,
        Vec2i,
        Vec2f
    >;

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
    operator size_t() const;

private:
    size_t type_hash;

    std::array<dparam, 4> param_data;
    unsigned param_count = 0;
    dparam  rdata;
};

struct dformat {
public:
    constexpr dformat(std::string_view t_name, dtype t_return_type, std::array<dtype, 4> t_params, unsigned t_param_count);

    // hash conversion
    constexpr operator size_t() const;

private:
    size_t type_hash;

    std::string name;
    std::array<dtype, 4> param_types;
    unsigned param_count = 0;
    dtype rtype;
};

template<StringLiteral Name, dtype RType, dtype... Params>
    requires (sizeof...(Params) <= 4)
struct dstatic_format : dformat {
    constexpr dstatic_format()
        : dformat{ Name, RType, { Params... }, sizeof...(Params) }
    {
    }

    constexpr
    std::tuple<to_type_t<Params>...>
    unwrap(const dmessage& msg) const;

    constexpr
    dmessage
    wrap(to_type_t<Params>...) const;

    constexpr
    dparam
    wrap_r(to_type_t<RType> r) const;

    constexpr
    to_type_t<RType>
    unwrap_r(dparam r) const;

};

dstatic_format dGetPosition = dstatic_format<"getpos", dtype::Vec2i>{};

}
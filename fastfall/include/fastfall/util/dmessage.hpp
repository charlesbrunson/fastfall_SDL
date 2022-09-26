#pragma once

#include <array>
#include <cinttypes>
#include <string>
#include <variant>
#include <stdexcept>

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

enum class dtype : uint8_t {
    Nil     = 0,
    Bool    = 1,
    Int     = 2,
    Float   = 3,
    Vec2i   = 4,
    Vec2f   = 5,
    // can go to 15 (4 bits)
};

using dvar = std::variant<
        std::monostate,
        bool,
        int,
        float,
        Vec2i,
        Vec2f
    >;

template<dtype T>
using to_type_t = std::variant_alternative_t<static_cast<uint64_t>(T), dvar>;

class dmessage {
public:
    constexpr dmessage
    (
        uint64_t t_hash,
        std::array<dvar, 4> t_params,
        unsigned t_count
    )
        : type_hash(t_hash)
        , param_data(t_params)
        , param_count(t_count)
    {
    }

    constexpr operator size_t() const { return type_hash; }
    constexpr uint64_t hash() const { return type_hash; }

    const std::array<dvar, 4>& params() const { return param_data; }
    unsigned pcount() const { return param_count; }

private:
    uint64_t type_hash;
    std::array<dvar, 4> param_data;
    unsigned param_count = 0;
};

struct dresult {
    bool accepted = false;
    std::optional<dvar> result = {};
};

template<StringLiteral Name, dtype RType = dtype::Nil, dtype... Params>
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

    // wraps message return param
    constexpr dresult
    accept(to_type_t<RType> r) const requires(RType != dtype::Nil) {
        return { true, r };
    }

    constexpr dresult
    accept() const requires(RType == dtype::Nil) {
        return { true, std::nullopt };
    }

    template<class Callable, class... Binds>
        requires std::is_invocable_v<Callable, Binds..., to_type_t<Params>...>
    constexpr dresult
    apply(const dmessage& msg, Callable&& callable, Binds&&... binds)  const {
        return { true, std::apply(callable, std::tuple_cat(std::tuple(binds...), unwrap(msg))) };
    };

    template<class Sendable>
    constexpr std::conditional_t<RType == dtype::Nil, bool, std::optional<to_type_t<RType>>>
    send(Sendable& mailbox, to_type_t<Params>... params) const {
        if constexpr (RType == dtype::Nil) {
            return unwrap_r(mailbox.message(wrap(std::forward<to_type_t<Params>>(params)...)));
        }
        else {
            return unwrap_r(mailbox.message(wrap(std::forward<to_type_t<Params>>(params)...)));
        }
    }

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
            throw std::invalid_argument{"message hash doesn't match format hash"};
        }
    }

    // wraps parameters into a message
    constexpr
    dmessage
    wrap(to_type_t<Params>... params) const
    {
        return { type_hash, { params... }, sizeof...(Params)};
    }

    // unwrap message return param
    constexpr std::conditional_t<RType == dtype::Nil, bool, std::optional<to_type_t<RType>>>
    unwrap_r(dresult r) const {
        if constexpr (RType != dtype::Nil) {
            if (r.accepted) {
                if (r.result && r.result->index() == dvar{to_type_t<RType>{}}.index()) {
                    return std::get<to_type_t<RType>>(*r.result);
                } else {
                    throw std::invalid_argument{"unexpected message return type"};
                }
            }
            else {
                return {};
            }
        }
        else {
            if (!r.result) {
                return r.accepted;
            } else {
                throw std::invalid_argument{"unexpected message return type, should be nil"};
            }
        }
    }
};

}
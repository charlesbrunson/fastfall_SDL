#pragma once

#include <array>
#include <tuple>
#include <cinttypes>
#include <string>
#include <variant>
#include <optional>
#include <stdexcept>
#include <algorithm>

namespace ff {
namespace detail {
    constexpr unsigned int hash(const char *str) {
        constexpr unsigned MULTIPLIER = 37u;
        unsigned int hash = 0u;
        for (auto *p = str; *p != '\0'; p++)
            hash = MULTIPLIER * hash + (unsigned char)*p;
        return hash;
    }

    template<class Variant, class T>
    struct index_of {
        constinit const static size_t value = Variant(T()).index();
    };

    template<class Variant, class T>
    constinit const size_t index_of_v = index_of<Variant, T>::value;

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
}

struct dvoid {};

template<class Variant, class... Binds>
    requires (std::variant_size_v<Variant> <= 16)
        && (std::same_as<std::variant_alternative_t<0, Variant>, dvoid>)
class dconfig {
public:
    class dmessage {
    public:
        constexpr dmessage (uint64_t t_hash, std::array<Variant, 4> t_params, unsigned t_count)
            : type_hash(t_hash), param_data(t_params), param_count(t_count)
        {
        }

        constexpr operator uint64_t() const { return type_hash; }

        constexpr uint64_t hash() const { return type_hash; }

        const std::array<Variant, 4> &params() const { return param_data; }

        unsigned pcount() const { return param_count; }

    private:
        uint64_t type_hash;
        std::array<Variant, 4> param_data;
        unsigned param_count = 0;
    };

    struct dresult {
        bool accepted = false;
        uint64_t type_hash = 0;
        std::optional<Variant> result = {};
    };

    static constexpr dresult reject = { false, {} };

    template<detail::StringLiteral Name, class RType = dvoid, class... Params>
        requires (sizeof...(Params) <= 4)
    class dformat {
    private:
        std::array<size_t, 4> param_types;

        //unsigned    param_count = 0;
        size_t      rtype;
        uint64_t    type_hash;

        constexpr static std::string_view type_name = Name;

    public:
        constexpr dformat()
            : rtype(detail::index_of_v<Variant, RType>)
            , param_types(std::array<size_t, 4>{ detail::index_of_v<Variant, Params>... })
            //, param_count(sizeof...(Params))
        {
            unsigned name_hash = detail::hash(Name.data());
            unsigned param_hash = 0;

            unsigned i = 0;
            for (auto &type: param_types) {
                param_hash |= (static_cast<uint8_t>(type) & 0b1111) << (4 * i);
                ++i;
            }
            param_hash |= (static_cast<uint8_t>(rtype) & 0b1111) << (4 * i);
            type_hash = ((uint64_t) name_hash << 32lu) | (uint64_t) param_hash;
        }

        constexpr operator uint64_t() const { return type_hash; }

        // wraps message return param
        constexpr dresult
        accept(RType r) const requires (detail::index_of_v<Variant, RType> != 0) {
            return {true, type_hash, r};
        }

        constexpr dresult
        accept() const requires (detail::index_of_v<Variant, RType> == 0) {
            return {true, type_hash, std::nullopt};
        }

        template<class Callable, class... CallBinds>
        requires std::is_invocable_r_v<
            std::conditional_t<detail::index_of_v<Variant, RType> != 0, RType, void>,
            Callable,
            CallBinds...,
            Params...
        >
        constexpr dresult
        apply(const dmessage &msg, Callable &&callable, CallBinds&&... binds) const {
            if constexpr (detail::index_of_v<Variant, RType> != 0) {
                return {true, type_hash, std::apply(std::forward<Callable>(callable), std::tuple_cat(std::tuple(std::forward<CallBinds>(binds)...), unwrap(msg)))};
            }
            else {
                std::apply(std::forward<Callable>(callable), std::tuple_cat(std::tuple(std::forward<CallBinds>(binds)...), unwrap(msg)));
                return {true, type_hash, std::nullopt};
            }
        };

        template<class Sendable>
        constexpr std::conditional_t<detail::index_of_v<Variant, RType> == 0, bool, std::optional<RType>>
        send(Sendable &mailbox, Binds... binds, Params... params) const {
            if constexpr (detail::index_of_v<Variant, RType> == 0) {
                return unwrap_result(mailbox.message(std::forward<Binds>(binds)..., wrap(std::forward<Params>(params)...)));
            } else {
                return unwrap_result(mailbox.message(std::forward<Binds>(binds)..., wrap(std::forward<Params>(params)...)));
            }
        }

        // unwrap message parameters
        constexpr
        std::optional<std::tuple<Params...>>
        unwrap(const dmessage &msg) const {
            if (msg.hash() == type_hash) {
                return [&]<size_t...Is>(std::index_sequence<Is...>) {
                    return std::make_tuple(
                            std::get<Params>(msg.params()[Is])...);
                }(std::make_index_sequence<sizeof...(Params)>{});
            } else {
                return std::nullopt;
            }
        }

        template<class U, size_t... I>
        requires std::constructible_from<U, Params...>
        constexpr std::optional<U>
        unwrap_as(const dmessage &msg) const {
            if (msg.hash() == type_hash) {
                return [&]<size_t...Is>(std::index_sequence<Is...>) {
                    return U{ std::get<Params>(msg.params()[Is])... };
                }(std::make_index_sequence<sizeof...(Params)>{});
            } else {
                return std::nullopt;
            }
        }


        // wraps parameters into a message
        constexpr
        dmessage
        wrap(const Params&... params) const {
            return {type_hash, { params... }, sizeof...(Params)};
        }

        // unwrap message return param
        constexpr
        std::conditional_t<detail::index_of_v<Variant, RType> == 0, bool, std::optional<RType>>
        unwrap_result(dresult r) const {
            if constexpr (detail::index_of_v<Variant, RType> == 0) {
                if (!r.result) {
                    return r.accepted && r.type_hash == type_hash;
                } else {
                    return false;
                }
            } else {
                if (r.accepted && r.result && r.type_hash == type_hash && std::holds_alternative<RType>(*r.result)) {
                    return std::get<RType>(*r.result);
                } else {
                    return {};
                }
            }
        }
    };
};

}

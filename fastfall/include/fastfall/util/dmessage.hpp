#pragma once

#include <array>
#include <cinttypes>
#include <string>
#include <variant>
#include <optional>
#include <stdexcept>

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
        constexpr static size_t value = Variant(T()).index();
    };

    template<class Variant, class T>
    constexpr size_t index_of_v = index_of<Variant, T>::value;

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


template<class Variant, class... Binds>
    requires (std::variant_size_v<Variant> <= 16)
        && (std::same_as<std::variant_alternative_t<0, Variant>, std::monostate>)
class dconfig {
public:
    class dmessage {
    public:
        constexpr dmessage
                (
                        uint64_t t_hash,
                        std::array<Variant, 4> t_params,
                        unsigned t_count
                )
                : type_hash(t_hash), param_data(t_params), param_count(t_count) {
        }

        constexpr operator size_t() const { return type_hash; }

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
        std::optional<Variant> result = {};
    };

    static constexpr dresult reject = { false, {} };

    template<detail::StringLiteral Name, class RType = std::monostate, class... Params>
        requires (sizeof...(Params) <= 4)
    class dformat {
    private:
        std::array<size_t, 4> param_types;
        unsigned param_count = 0;
        size_t rtype;

        uint64_t type_hash;

        constexpr static std::string_view type_name = Name;

    public:
        constexpr dformat()
            : rtype(detail::index_of_v<Variant, RType>)
            , param_types(std::array<size_t, 4>{ detail::index_of_v<Variant, Params>... })
            , param_count(sizeof...(Params))
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
            return {true, r};
        }

        constexpr dresult
        accept() const requires (detail::index_of_v<Variant, RType> == 0) {
            return {true, std::nullopt};
        }

        template<class Callable, class... Binds>
        requires std::is_invocable_v<Callable, Binds..., Params...>
        constexpr dresult
        apply(const dmessage &msg, Callable &&callable, Binds &&... binds) const {
            return {true, std::apply(callable, std::tuple_cat(std::tuple(binds...), unwrap(msg)))};
        };

        template<class Sendable>
        constexpr std::conditional_t<detail::index_of_v<Variant, RType> == 0, bool, std::optional<RType>>
        send(Sendable &mailbox, Binds... binds, Params... params) const {
            if constexpr (detail::index_of_v<Variant, RType> == 0) {
                return unwrap_r(mailbox.message(std::forward<Binds>(binds)..., wrap(std::forward<Params>(params)...)));
            } else {
                return unwrap_r(mailbox.message(std::forward<Binds>(binds)..., wrap(std::forward<Params>(params)...)));
            }
        }

        // unwrap message parameters
        constexpr
        std::tuple<Params...>
        unwrap(const dmessage &msg) const {
            if (msg.hash() == type_hash) {
                // what fresh hell is this
                return [&]<uint64_t...Is>(std::index_sequence<Is...>) {
                    return std::make_tuple(
                            std::get<Params>(msg.params()[Is])...);
                }(std::make_index_sequence<sizeof...(Params)>{});
            } else {
                throw std::invalid_argument{"message hash doesn't match format hash"};
            }
        }

        // wraps parameters into a message
        constexpr
        dmessage
        wrap(const Params&... params) const {
            return {type_hash, { params... }, sizeof...(Params)};
        }

        // unwrap message return param
        constexpr std::conditional_t<detail::index_of_v<Variant, RType> == 0, bool, std::optional<RType>>
        unwrap_r(dresult r) const {
            if constexpr (detail::index_of_v<Variant, RType> != 0) {
                if (r.accepted) {
                    if (r.result && std::holds_alternative<RType>(*r.result)) {
                        return std::get<RType>(*r.result);
                    } else {
                        throw std::invalid_argument{"unexpected message return type"};
                    }
                } else {
                    return {};
                }
            } else {
                if (!r.result) {
                    return r.accepted;
                } else {
                    throw std::invalid_argument{"unexpected message return type, should be nil"};
                }
            }
        }
    };
};

}
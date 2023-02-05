#pragma once

#include "fastfall/util/slot_map.hpp"

#include <concepts>

namespace ff {


template<class T>
struct ID {
	slot_key value;

	bool operator==(const ID<T>& other) const { return value == other.value; };
	bool operator!=(const ID<T>& other) const { return value != other.value; };
	bool operator<(const ID<T>& other)  const {
        return value.sparse_index != other.value.sparse_index
            ? value.sparse_index < other.value.sparse_index
            : value.generation < other.value.generation;
    };

    explicit operator bool() const {
        return (bool)value;
    }

	template<class Base>
		requires std::derived_from<T, Base>
	operator ID<Base>() const {
		return { value };
	}

    size_t raw() const {
        return value.raw();
    }
};

template<class Other, class Base>
requires std::derived_from<Other, Base>
ID<Other> id_cast(ID<Base> id) {
	return ID<Other>{ id.value };
}

template<class T>
struct ID_ptr {
    ID<T> id;
    T* ptr;

    template<class Base>
    requires std::derived_from<T, Base>
    operator ID<Base>() const { return id; }

    T* get() { return ptr; }
    const T* get() const { return ptr; }

    T& operator* () { return *ptr; }
    T* operator->() { return ptr; }
    const T& operator* () const { return *ptr; }
    const T* operator->() const { return ptr; }
};

}

struct id_placeholder_t {};
static id_placeholder_t id_placeholder;

auto&& set_placeholder_id(auto&& arg, auto&& id) {
    if constexpr (std::same_as<id_placeholder_t, std::remove_cvref_t<decltype(arg)>>)
    {
        return std::forward<decltype(id)>(id);
    }
    else {
        return std::forward<decltype(arg)>(arg);
    }
}


template<typename T, typename ID>
using swap_id_t = std::conditional_t<std::same_as<std::remove_cvref_t<T>, id_placeholder_t>, ID, T>;

namespace std {

template<class T>
struct hash<ff::ID<T>> {
    size_t operator()(const ff::ID<T> &x) const {
        return hash<size_t>()(x.value.raw());
    }
};

}


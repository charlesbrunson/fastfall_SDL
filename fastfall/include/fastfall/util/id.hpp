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
	operator ID<Base>() {
		return { value };
	}

    size_t raw() const {
        return value.raw();
    }
};

template<class Other, class Base>
//requires std::derived_from<Derived, Base>
ID<Other> id_cast(ID<Base> id) {
	return { id.value };
}

}

namespace std {

template<class T>
struct hash<ff::ID<T>> {
    size_t operator()(const ff::ID<T> &x) const {
        return hash<size_t>()(x.value.raw());
    }
};

}


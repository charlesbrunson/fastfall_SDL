#pragma once

#include "fastfall/util/slot_map.hpp"

namespace ff {


struct GenericID {
	size_t type_hash;
	slot_key value;

	bool operator==(const GenericID& other) const { return type_hash == other.type_hash && value == other.value; };
	bool operator!=(const GenericID& other) const { return type_hash != other.type_hash || value != other.value; };
	bool operator<(const GenericID& other)  const { 
		return (type_hash == other.type_hash)
			? value.sparse_index < other.value.sparse_index
			: type_hash < other.type_hash;
	};
};

template<class T>
struct ID {
	slot_key value;
	bool operator==(const ID<T>& other) const { return value == other.value; };
	bool operator!=(const ID<T>& other) const { return value != other.value; };
	bool operator<(const ID<T>& other)  const { return value.sparse_index < other.value.sparse_index; };

	operator GenericID() {
		return { typeid(T).hash_code(), value};
	}
};

}

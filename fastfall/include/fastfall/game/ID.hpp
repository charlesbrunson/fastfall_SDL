#pragma once

#include "fastfall/util/slot_map.hpp"

namespace ff {

struct notype {};

template<class T>
struct ID {
	size_t type_hash; // contains the base type of the component, equals the type_hash of the slot_map
	slot_key value;   // key to the component in the slot_map

	template<class U>
	bool operator==(const ID<U>& other) const { return type_hash == other.type_hash && value == other.value; };

	template<class U>
	bool operator!=(const ID<U>& other) const { return type_hash != other.type_hash || value != other.value; };

	template<class U>
	bool operator<(const ID<U>& other)  const 
	{ 
		if (type_hash == other.type_hash) {
			return value.sparse_index < other.value.sparse_index;
		}
		else {
			return type_hash < other.type_hash;
		}
	};

	operator ID<notype>() {
		return { type_hash, value };
	}

	size_t raw() const { return *reinterpret_cast<const size_t*>(this); }
};

using GenericID = ID<notype>;

template<class T>
std::optional<ID<T>> id_cast(GenericID gen)
{
	return gen.type_hash == typeid(T).hash_code() 
		? std::make_optional<ID<T>>(gen.type_hash, gen.value) 
		: std::nullopt;
}

}

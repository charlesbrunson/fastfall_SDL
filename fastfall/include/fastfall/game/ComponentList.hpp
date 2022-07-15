#pragma once

#include "fastfall/util/slot_map.hpp"
#include "fastfall/util/copyable_uniq_ptr.hpp"
#include "fastfall/game/ID.hpp"

#include <vector>
#include <memory>
#include <concepts>
#include <functional>

namespace ff {

template<class T, bool Polymorphic>
class ComponentList
{
private:
	using storage_type = std::conditional_t<Polymorphic, copyable_unique_ptr<T>, T>;
	slot_map<storage_type> components;

	static inline size_t type_hash = typeid(T).hash_code();

public:
	using type = T;
	using is_poly = std::bool_constant<Polymorphic>;

	// non-polymorphic
	template<class... Args, class = std::enable_if_t<!Polymorphic>>
	ID<T> create(Args&&... args) {
		auto key = components.emplace_back(std::forward<Args>(args)...);
		ID<T> id{ type_hash, key };
		on_create(id);
		return id;
	}

	template<class = std::enable_if_t<!Polymorphic>>
	T& at(ID<T> id) {
		assert(id.type_hash == type_hash);
		return components.at(id.value);
	}

	template<class = std::enable_if_t<!Polymorphic>>
	const T& at(ID<T> id) const {
		assert(id.type_hash == type_hash);
		return components.at(id.value);
	}

	template<class = std::enable_if_t<!Polymorphic>>
	bool erase(ID<T> id) {
		bool removed = exists(id);
		if (removed) {
			components.erase(id.value);
			on_erase(id);
		}
		return removed;
	}

	template<class = std::enable_if_t<!Polymorphic>>
	bool exists(ID<T> id)
	{
		return id.type_hash == type_hash && components.exists(id.value);
	}

	template<class = std::enable_if_t<!Polymorphic>>
	ID<T> id(const T& component)
	{
		return { type_hash, *components.key_of(component) };
	}

	// polymorphic
	template<std::derived_from<T> Type, class... Args, class = std::enable_if_t<Polymorphic>>
	ID<Type> create(Args&&... args) {
		auto key = components.emplace_back(std::make_unique<Type>(std::forward<Args>(args)...));
		ID<Type> id{ type_hash, key };
		on_create(id);
		return id;
	}

	template<std::derived_from<T> Type, class = std::enable_if_t<Polymorphic>>
	Type& at(ID<Type> id) {
		assert(id.type_hash == type_hash);
		return *reinterpret_cast<Type*>(components.at(id.value).get());
	}

	template<std::derived_from<T> Type, class = std::enable_if_t<Polymorphic>>
	const Type& at(ID<Type> id) const {
		assert(id.type_hash == type_hash);
		return *reinterpret_cast<const Type*>(components.at(id.value).get());
	}

	template<std::derived_from<T> Type>
	bool erase(ID<Type> id) {
		bool removed = exists(id);
		if (removed) {
			components.erase(id.value);
			on_erase(id);
		}
		return removed;
	}

	template<std::derived_from<T> Type>
	bool exists(ID<Type> id)
	{
		return id.type_hash == type_hash && components.exists(id.value);
	}

	template<std::derived_from<T> Type, class = std::enable_if_t<Polymorphic>>
	ID<Type> id(const Type& component)
	{
		auto it = std::find_if(components.begin(), components.end(), [&](const storage_type& st) {
			return &component == st.get();
		});

		return { type_hash, *components.key_of(it) };
	}

	inline auto begin() { return components.begin(); }
	inline auto begin() const { return components.begin(); }
	inline auto cbegin() const { return components.begin(); }

	inline auto end() { return components.end(); }
	inline auto end() const { return components.end(); }
	inline auto cend() const { return components.cend(); }
	
};


}
#pragma once

#include "fastfall/util/slot_map.hpp"
#include "fastfall/game/ID.hpp"

#include <vector>
#include <memory>
#include <concepts>

namespace ff {

template<class T, bool Polymorphic = false>
class ComponentList
{
private:
	using base_type = T;
	using value_type = std::conditional_t<Polymorphic, std::unique_ptr<T>, T>;

	slot_map<value_type> components;

public:

	// non-polymorphic
	template<class... Args, class = std::enable_if_t<!Polymorphic>>
	ID<T> create(Args&&... args) {
		return { components.emplace_back(std::forward<Args>(args)...) };
	}

	template<class = std::enable_if_t<!Polymorphic>>
	T& get(ID<T> id) {
		return components.at(id.value);
	}

	template<class = std::enable_if_t<!Polymorphic>>
	bool erase(ID<T> id) {
		bool removed = exists(id);
		components.erase(id.value);
		return removed;
	}

	template<class = std::enable_if_t<!Polymorphic>>
	bool exists(ID<T> id)
	{
		return components.exists(id.value);
	}

	// polymorphic
	template<std::derived_from<T> Type, class... Args, class = std::enable_if_t<Polymorphic>>
	ID<Type> create(Args&&... args) {
		return { components.emplace_back(std::make_unique<Type>(std::forward<Args>(args)...)) };
	}

	template<std::derived_from<T> Type, class = std::enable_if_t<Polymorphic>>
	Type& get(ID<Type> id) {
		return components.at(id.value);
	}

	template<std::derived_from<T> Type>
	bool erase(ID<Type> id) {
		bool removed = exists(id);
		components.erase(id.value);
		return removed;
	}

	template<std::derived_from<T> Type>
	bool exists(ID<Type> id)
	{
		return components.exists(id.value);
	}

	inline auto begin() { return components.begin(); }
	inline auto begin() const { return components.begin(); }
	inline auto cbegin() const { return components.begin(); }

	inline auto end() { return components.end(); }
	inline auto end() const { return components.end(); }
	inline auto cend() const { return components.cend(); }

	std::function<void()> on_create;
	std::function<void()> on_erase;
};


}
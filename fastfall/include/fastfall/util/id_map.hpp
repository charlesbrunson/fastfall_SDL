#pragma once

#include "fastfall/util/slot_map.hpp"
#include "fastfall/util/copyable_uniq_ptr.hpp"
#include "fastfall/util/id.hpp"

#include <span>
#include <vector>
#include <memory>
#include <concepts>
#include <functional>

namespace ff {



template<class T>
class id_map
{
private:
	using base_type = T;
	using value_type = T;

	slot_map<value_type> components;

public:
    using span = std::span<value_type>;

	template<class... Args>
	ID<T> create(Args&&... args) {
		auto id = components.emplace_back(std::forward<Args>(args)...);
		return { id };
	}

	T& at(ID<T> id) {
		return components.at(id.value);
	}

	const T& at(ID<T> id) const {
		return components.at(id.value);
	}

    T* get(ID<T> id) {
        return exists(id) ? &at(id) : nullptr;
    }

    const T* get(ID<T> id) const {
        return exists(id) ? &at(id) : nullptr;
    }

	bool erase(ID<T> id) {
		bool removed = exists(id);
		if (removed) {
			components.erase(id.value);
		}
		return removed;
	}

	bool exists(ID<T> id)
	{
		return components.exists(id.value);
	}

    size_t size() const { return components.size(); }

	inline auto begin() { return components.begin(); }
	inline auto begin() const { return components.begin(); }
	inline auto cbegin() const { return components.begin(); }

	inline auto end() { return components.end(); }
	inline auto end() const { return components.end(); }
	inline auto cend() const { return components.cend(); }

    ID<T> id_of(const value_type& value) const {
       return { *components.key_of(value) };
    }

    ID<T> id_of(typename slot_map<value_type>::const_iterator iter) const {
        return { *components.key_of(iter) };
    }
};

template<class T>
class poly_id_map
{
private:
	using base_type = T;
	using value_type = copyable_unique_ptr<T>;

	slot_map<value_type> components;

public:
    using span = std::span<value_type>;

	// polymorphic
	template<std::derived_from<T> Type, class... Args>
	ID<Type> create(Args&&... args) {
		auto id = components.emplace_back(std::make_unique<Type>(std::forward<Args>(args)...));
		return { id };
	}

	template<std::derived_from<T> Type>
	Type& at(ID<Type> id) {
		return *reinterpret_cast<Type*>(components.at(id.value).get());
	}

	template<std::derived_from<T> Type>
	const Type& at(ID<Type> id) const {
		return *reinterpret_cast<const Type*>(components.at(id.value).get());
	}

    template<std::derived_from<T> Type>
    Type* get(ID<Type> id) {
        return exists(id) ? &at(id) : nullptr;
    }

    template<std::derived_from<T> Type>
    const Type* get(ID<Type> id) const {
        return exists(id) ? &at(id) : nullptr;
    }

	template<std::derived_from<T> Type>
	bool erase(ID<Type> id) {
		bool removed = exists(id);
		if (removed) {
			components.erase(id.value);
		}
		return removed;
	}

	template<std::derived_from<T> Type>
	bool exists(ID<Type> id)
	{
		return components.exists(id.value);
	}

    size_t size() const { return components.size(); }

	inline auto begin() { return components.begin(); }
	inline auto begin() const { return components.begin(); }
	inline auto cbegin() const { return components.begin(); }

	inline auto end() { return components.end(); }
	inline auto end() const { return components.end(); }
	inline auto cend() const { return components.cend(); }

    ID<T> id_of(const value_type& value) const {
        return { *components.key_of(value) };
    }
    ID<T> id_of(typename slot_map<value_type>::const_iterator iter) const {
        return { components.key_of(iter) };
    }
};


}
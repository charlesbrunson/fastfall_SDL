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


template<class T, class Ptr>
class id_iterator_base
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Ptr;

    id_iterator_base() = default;
    explicit id_iterator_base(value_type ptr) : m_ptr(ptr) {}

    id_iterator_base& operator=(value_type ptr) { m_ptr = ptr; return *this; }
    id_iterator_base& operator=(const id_iterator_base& other) { m_ptr = other.ptr; return *this; }

    auto operator*() requires(!std::is_const_v<Ptr>) { return std::make_pair( ID<T>{m_ptr->first}, std::ref(m_ptr->second) ); }
    auto operator*() const { return std::make_pair( ID<T>{m_ptr->first}, std::ref(m_ptr->second) ); }

    auto operator->() requires(!std::is_const_v<Ptr>) { return &m_ptr->second; }
    auto operator->() const { return &m_ptr->second; }

    ID<T> id() const { return {m_ptr->first}; }

    auto& value() { return m_ptr->second; }
    const auto& value() const { return m_ptr->second; }

    int operator<=>(const id_iterator_base& other) const { return m_ptr <=> other.m_ptr; }
    int operator!=(const id_iterator_base& other) const { return m_ptr != other.m_ptr; }

    id_iterator_base& operator++() { ++m_ptr; return *this; }
    id_iterator_base operator++(int) { id_iterator_base it{*this}; ++(*this); return it; }

    id_iterator_base operator--() { --m_ptr; return *this; }
    id_iterator_base operator--(int) { id_iterator_base it{*this}; --(this); return it; }

    id_iterator_base operator+(difference_type movement) const { auto it = *this; it.m_ptr + movement; return it; }
    id_iterator_base operator-(difference_type movement) const { auto it = *this; it.m_ptr - movement; return it; }

    difference_type operator-(const id_iterator_base& other) const { return std::distance(m_ptr, other.m_ptr); }

private:
    value_type m_ptr = nullptr;
};

template<class T>
using id_iterator = id_iterator_base<T, std::pair<slot_key, T>*>;

template<class T>
using const_id_iterator = id_iterator_base<T, const std::pair<slot_key, T>*>;

template<class T>
class id_map
{
private:
	using base_type = T;
	using value_type = T;

	slot_map<value_type> components;

public:
    using span = std::span<value_type>;

    using iterator = id_iterator<T>;
    using const_iterator = const_id_iterator<T>;

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

	bool exists(ID<T> id) const
	{
		return components.exists(id.value);
	}

    bool empty() const {
        return size() == 0;
    }

    size_t size() const { return components.size(); }

	inline auto begin() { return iterator{ components.data() }; }
	inline auto begin() const { return const_iterator{ components.data() }; }
	inline auto cbegin() const { return const_iterator{ components.data() }; }

	inline auto end() { return iterator{ components.data() + components.size() }; }
	inline auto end() const { return const_iterator{ components.data() + components.size() }; }
	inline auto cend() const { return const_iterator{ components.data() + components.size() }; }

    ID<T> id_of(const value_type& value) const {
       return { *components.key_of(value) };
    }

    ID<T> id_of(typename slot_map<value_type>::const_iterator iter) const {
        return { *components.key_of(iter) };
    }

    ID<T> peek_next_id() const { return { components.peek_next_key() }; }
};

template<class T>
class poly_id_map
{
public:
    using base_type = T;
    using value_type = copyable_unique_ptr<T>;
    using span = std::span<value_type>;

private:
	slot_map<value_type> components;

public:
    using iterator = id_iterator<T>;
    using const_iterator = const_id_iterator<T>;

	// polymorphic
	template<std::derived_from<T> Type, class... Args>
	ID<Type> create(Args&&... args) {
		auto id = components.emplace_back(make_copyable_unique<T, Type>(std::forward<Args>(args)...));
		return { id };
	}

    ID<T> emplace(value_type&& val) {
        auto id = components.emplace_back(std::move(val));
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

    ID<T> peek_next_id() const { return { components.peek_next_key() }; }
};


}
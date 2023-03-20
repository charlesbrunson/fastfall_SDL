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


template<class T, class UnderID = T>
class id_iterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using base_type         = std::remove_const_t<T>;
    using pair_type         = std::pair<slot_key, base_type>;
    using value_type        = std::conditional_t<std::is_const_v<T>, const pair_type*, pair_type*>;
    using id_type           = ID<UnderID>;

    id_iterator() = default;
    explicit id_iterator(value_type ptr) : m_ptr(ptr) {}

    id_iterator& operator=(value_type ptr) { m_ptr = ptr; return *this; }
    id_iterator& operator=(const id_iterator& other) { m_ptr = other.ptr; return *this; }

    auto operator*() requires(!std::is_const_v<T>) { return std::make_pair( id_type{m_ptr->first}, std::ref(m_ptr->second) ); }
    auto operator*() const { return std::make_pair( id_type{m_ptr->first}, std::ref(m_ptr->second) ); }

    auto operator->() requires(!std::is_const_v<T>) { return &m_ptr->second; }
    auto operator->() const { return &m_ptr->second; }

    id_type id() const { return {m_ptr->first}; }

    auto& value() { return m_ptr->second; }
    const auto& value() const { return m_ptr->second; }

    int operator<=>(const id_iterator& other) const { return m_ptr <=> other.m_ptr; }
    int operator!=(const id_iterator& other) const { return m_ptr != other.m_ptr; }

    id_iterator& operator++() { ++m_ptr; return *this; }
    id_iterator operator++(int) { id_iterator it{*this}; ++(*this); return it; }

    id_iterator operator--() { --m_ptr; return *this; }
    id_iterator operator--(int) { id_iterator it{*this}; --(this); return it; }

    id_iterator operator+(difference_type movement) const { auto it = *this; it.m_ptr + movement; return it; }
    id_iterator operator-(difference_type movement) const { auto it = *this; it.m_ptr - movement; return it; }

    difference_type operator-(const id_iterator& other) const { return std::distance(m_ptr, other.m_ptr); }

private:
    value_type m_ptr = nullptr;
};

template<class T>
class id_map
{
public:
	using base_type = T;
	using value_type = T;
    constexpr static bool is_poly = false;

    template<typename Item>
    constexpr static bool fits() { return std::same_as<Item, T>; };

private:
    slot_map<value_type> components;

public:
    using span = std::span<value_type>;

    using iterator = id_iterator<value_type>;
    using const_iterator = id_iterator<const value_type>;

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
    constexpr static bool is_poly = true;

    template<typename Item>
    constexpr static bool fits() { return std::derived_from<Item, T>; };

private:
	slot_map<value_type> components;

public:
    using iterator = id_iterator<value_type, base_type>;
    using const_iterator = id_iterator<const value_type, base_type>;

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

    template<std::derived_from<T> Type, class... Args>
    void emplace_at(ID<T> id, Args&&... args) {
        components.at(id.value) = make_copyable_unique<T, Type>(std::forward<Args>(args)...);
    }

    void emplace_at(ID<T> id, value_type&& val) {
        components.at(id.value) = std::move(val);
    }

	template<std::derived_from<T> Type>
	Type& at(ID<Type> id) {
		return *static_cast<Type*>(components.at(id.value).get());
	}

	template<std::derived_from<T> Type>
	const Type& at(ID<Type> id) const {
		return *static_cast<const Type*>(components.at(id.value).get());
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
	bool exists(ID<Type> id) const
	{
		return components.exists(id.value);
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
        return { components.key_of(iter) };
    }

    ID<T> peek_next_id() const { return { components.peek_next_key() }; }
};


}
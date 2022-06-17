#pragma once

#include <limits>
#include <cstdint>
#include <memory>
#include <array>
#include <utility>
#include <cassert>

namespace ff {

/**
 * A bad implementation of a slot map over a contiguous array
 * Requires C++20
 */
struct slot_key {
	const uint32_t value = 0;
	bool operator==(const slot_key& other) const noexcept { return value == other.value; }
	bool operator!=(const slot_key& other) const noexcept { return value != other.value; }

	operator uint32_t() const { return value; }
};

template<typename T, size_t Count, bool HeapAlloc = true>
class slot_array
{
	static_assert(Count <= std::numeric_limits<uint16_t>::max() - 1, "Capacity may not be greater than (2 ^ 16) - 1");

	constexpr static uint16_t LIST_HEAD = std::numeric_limits<uint16_t>::max();
	constexpr static uint16_t LIST_TAIL = Count;

private:
	struct element
	{
		constexpr element()
			: m_exists(0)
			, m_gen(0)
			, m_index(0)
			, m_next(0)
			, m_prev(0)
		{
		};

		constexpr element(const element& other)
			: m_gen(other.m_gen)
			, m_index(other.m_index)
			, m_next(other.m_next)
			, m_prev(other.m_prev)
		{
			if (other.m_exists)
				construct(other.get());
		}

		constexpr element(element&& other)
			: m_gen(other.m_gen)
			, m_index(other.m_index)
			, m_next(other.m_next)
			, m_prev(other.m_prev)
		{
			if (other.m_exists)
				construct(std::move(other.get()));
		}

		constexpr element& operator=(const element& other)
		{
			if (exists())
				destroy();

			m_gen = other.m_gen;
			m_index = other.m_index;
			m_next = other.m_next;
			m_prev = other.m_prev;

			if (other.m_exists)
				construct(other.get());

			return *this;
		}

		constexpr element& operator=(element&& other)
		{
			if (exists())
				destroy();

			m_gen = other.m_gen;
			m_index = other.m_index;
			m_next = other.m_next;
			m_prev = other.m_prev;

			if (other.m_exists)
				construct(std::move(other.get()));

			return *this;
		}

		constexpr ~element() {
			if (exists()) {
				destroy();
			}
		};

		constexpr bool		exists()		const noexcept { return m_exists; }
		constexpr uint16_t	index()			const noexcept { return m_index; }
		constexpr uint16_t	generation()	const noexcept { return m_gen; }

		constexpr slot_key key() const noexcept {
			return slot_key{ (uint32_t)m_exists << 31 | m_gen << 16 | m_index };
		}

		constexpr uint16_t next() const noexcept { return m_next; }
		constexpr uint16_t prev() const noexcept { return m_prev; }

		constexpr T& get() { return *ptr(); }
		constexpr const T& get() const { return *ptr(); }

		constexpr T* operator->() { return ptr(); }
		constexpr const T* operator->() const { return ptr(); }

	private:

		/* where our T may or may not live */
		std::aligned_storage_t<sizeof(T), alignof(T)> slot;

		uint16_t m_exists : 1;	/* whether something lives in object */
		uint16_t m_gen : 15;	/* mostly unique value to seperate objects that have been here */
		uint16_t m_index;		/* this element's index in the array */

		uint16_t m_next;		/* if m_exists, index of the next existing, else the next free */
		uint16_t m_prev;		/* if m_exists, index of the previous existing, else the previous free */

		template<typename... Args>
			requires std::is_constructible_v<T, Args...>
		void construct(Args&&... args)
		{
			assert(!exists());
			std::construct_at(ptr(), std::forward<Args>(args)...);
			m_exists = true;
		}

		void replace(T&& value)
		{
			*ptr() = std::forward<T>(value);
			incr_gen();
		}

		void replace(const T& value)
		{
			*ptr() = value;
			incr_gen();
		}

		void destroy()
		{
			assert(exists());
			std::destroy_at(ptr());
			m_exists = false;
			incr_gen();
		}

		constexpr T* ptr() noexcept {
			return std::launder(reinterpret_cast<T*>(&slot));
		}
		constexpr const T* ptr() const noexcept {
			return std::launder(reinterpret_cast<const T*>(&slot));
		}

		void incr_gen() noexcept {
			m_gen = (m_gen + 1) % 0x7FFF;
		}

		friend class slot_array;
	};

public:

	using value_type = T;

	using pointer = T*;
	using reference = T&;

	using const_pointer = const T*;
	using const_reference = const T&;

	using size_type = uint16_t;
	using difference_type = ptrdiff_t;

public:

	template<typename T, typename T_Ptr, typename T_Ref>
	class SlotArrayIterator
	{
		element* elem;

	public:
		explicit constexpr SlotArrayIterator(element* ptr = nullptr)
			: elem(ptr)
		{
		}

		constexpr SlotArrayIterator(const SlotArrayIterator& iter)
			: elem(iter.elem)
		{
		}

		constexpr T_Ptr operator->() const {
			return &elem->get();
		}


		constexpr T_Ref operator*() const {
			return elem->get();
		}

		constexpr SlotArrayIterator& operator++()
		{
			assert(elem->m_index < Count);
			elem += (elem->next() - elem->index());
			return *this;
		}

		constexpr SlotArrayIterator& operator++(int)
		{
			assert(elem->m_index < Count);
			auto tmp = *this;
			elem += (elem->next() - elem->index());
			return tmp;
		}

		constexpr SlotArrayIterator operator+(size_type other)
		{
			auto it = *this;
			while (other-- > 0)
			{
				it.operator++();
			}
			return it;
		}

		constexpr SlotArrayIterator& operator--()
		{
			assert(elem->m_index > 0);
			elem -= (elem->index() - elem->prev());
			return *this;
		}

		constexpr SlotArrayIterator& operator--(int)
		{
			assert(elem->m_index > 0);
			auto tmp = *this;
			elem -= (elem->index() - elem->prev());
			return tmp;
		}


		constexpr SlotArrayIterator operator-(size_type other)
		{
			auto it = *this;
			while (other-- > 0)
			{
				it.operator--();
			}
			return it;
		}

		T_Ref get() const {
			return elem->get();
		}

		template<typename Other_Ptr, typename Other_Ref>
		constexpr bool operator==(const SlotArrayIterator<T, Other_Ptr, Other_Ref>& other) const noexcept {
			return elem == other.elem;
		}

		template<typename Other_Ptr, typename Other_Ref>
		constexpr bool operator!=(const SlotArrayIterator<T, Other_Ptr, Other_Ref>& other) const noexcept {
			return elem != other.elem;
		}

		constexpr slot_key key() const noexcept {
			assert(elem->m_index != Count);
			return elem->key();
		}

		friend class slot_array;
	};

	using iterator = SlotArrayIterator<T, T*, T&>;
	using const_iterator = SlotArrayIterator<T, const T*, const T&>;

public:

	// constructors
	constexpr slot_array()
	{
		init_storage();
	}

	constexpr slot_array(const slot_array& other)
	{
		init_storage();
		assign(other);
	}

	constexpr slot_array(size_type count, const T& value = T())
	{
		init_storage();
		assign(count, value);
	}

	template<class InputIt>
	constexpr slot_array(InputIt first, InputIt last)
	{
		init_storage();
		assign(first, last);
	}

	constexpr slot_array(std::initializer_list<T> init)
	{
		init_storage();
		assign(init);
	}

	// destructor
	constexpr ~slot_array()
	{
		clear();

	}

	slot_array& operator= (const slot_array& other)
	{
		assign(other);
		return *this;
	}

	slot_array& operator= (slot_array&& other)
	{
		swap(other);
		return *this;
	}


	// assignment

	constexpr void assign(const slot_array& other)
	{
		for (size_t i = 0; i < Count; i++)
		{
			data()[i] = other.data()[i];
		}
		m_size = other.m_size;
		m_live_head = other.m_live_head;
		m_live_tail = other.m_live_tail;
		m_free_head = other.m_free_head;
	}

	constexpr void assign(slot_array&& other)
	{
		swap(other);
	}

	constexpr void assign(size_type count, const T& value = T())
	{
		clear();
		insert(begin(), count, value);
	}

	template<class InputIt>
	constexpr void assign(InputIt first, InputIt last)
	{
		clear();
		insert(begin(), first, last);
	}

	constexpr void assign(std::initializer_list<T> init)
	{
		clear();
		insert(begin(), init.begin(), init.end());
	}

	// accessors

	constexpr reference front() { return data()[m_live_head].get(); }
	constexpr const_reference front() const { return data()[m_live_head].get(); }

	constexpr reference back() { return data()[m_live_tail].get(); }
	constexpr const_reference back() const { return data()[m_live_tail].get(); }

	constexpr element* data() noexcept {
		if constexpr (!HeapAlloc) {
			return &m_data[0];
		}
		else {
			return &m_data.get()[0];
		}
	};
	constexpr const element* data() const noexcept {
		if constexpr (!HeapAlloc) {
			return &m_data[0];
		}
		else {
			return &m_data.get()[0];
		}
	};

	constexpr iterator find(slot_key key) {
		element* it = &data()[key.value & std::numeric_limits<uint16_t>::max()];
		if (it->exists() && it->key() == key) {
			return iterator{ it };
		}
		else {
			return end();
		}
	}
	constexpr const_iterator find(slot_key key) const {
		element* it = &data()[key.value() & std::numeric_limits<uint16_t>::max()];
		if (it->exists() && it->key() == key.value) {
			return iterator{ it };
		}
		else {
			return end();
		}
	}

	// iterators

	constexpr iterator begin() { return iterator{ data() + m_live_head }; };
	constexpr const_iterator begin() const { return const_iterator{ data() + m_live_head }; };
	constexpr const_iterator cbegin() const { return const_iterator{ data() + m_live_head }; };

	constexpr iterator end() { return iterator{ data() + capacity() }; };
	constexpr const_iterator end() const { return const_iterator{ data() + capacity() }; };
	constexpr const_iterator cend() const { return const_iterator{ data() + capacity() }; };

	//constexpr std::span<element, Count> walk() { return  std::span<element, Count>{ data(), Count }; }
	//constexpr std::span<const element, Count> walk() const { return  std::span<const element, Count>{ data(), Count }; }
	//constexpr std::span<const element, Count> cwalk() const { return  std::span<const element, Count>{ data(), Count }; }

	// capacity

	constexpr bool empty() const noexcept { return m_size == 0; };
	constexpr bool full()  const noexcept { return m_size == capacity(); }

	constexpr size_type size() const noexcept { return m_size; };
	constexpr size_type max_size() const noexcept { return Count; }
	constexpr size_type capacity() const noexcept { return Count; }

	// modifiers

	// modifiers - clear
	constexpr void clear() noexcept
	{
		while (begin() != end())
		{
			erase(begin());
		}
		init_lists();
	}

	// modifiers - insert
	constexpr iterator insert(iterator pos, const T& value)
	{
		if (full())	throw std::bad_alloc();

		auto ndx = free_pop();
		data()[ndx].construct(value);
		live_insert(ndx, static_cast<size_type>(pos.elem - data()));
		m_size++;
		return iterator{ data() + ndx };
	};

	constexpr iterator insert(iterator pos, T&& value)
	{
		if (full())	throw std::bad_alloc();

		auto ndx = free_pop();
		data()[ndx].construct(std::forward<T>(value));
		live_insert(ndx, static_cast<size_type>(pos.elem - data()));
		m_size++;
		return iterator{ data() + ndx };
	};

	constexpr iterator insert(iterator pos, size_type count, const T& value)
	{
		if (count > 0)
		{
			auto it = pos;
			pos = insert(pos, value);
			for (; count > 0; count--)
			{
				insert(it, value);
			}
		}
		return pos;
	};

	template<class InputIt>
	constexpr iterator insert(iterator pos, InputIt first, InputIt last)
	{
		if (first != last)
		{
			auto it = pos;
			pos = insert(pos, *first++);
			for (; first != last; ++first)
			{
				insert(it, *first);
			}
		}
		return pos;
	};

	constexpr iterator insert(iterator pos, std::initializer_list<T> ilist)
	{
		if (ilist.size() > 0)
		{
			auto beg = ilist.begin();
			auto it = pos;
			pos = insert(pos, *beg++);
			for (; beg != ilist.end(); ++beg)
			{
				insert(it, *beg);
			}
		}
		return pos;
	};

	// modifiers - emplace
	template<class... Args>
	constexpr iterator emplace(iterator pos, Args&&... args)
	{
		if (full())	throw std::bad_alloc();

		auto ndx = free_pop();
		data()[ndx].construct(std::forward<Args>(args)...);
		live_insert(ndx, pos.elem - data());
		m_size++;
		return iterator{ data() + ndx };
	};

	// modifiers - erase
	constexpr iterator erase(iterator pos)
	{
		auto ndx = pos.elem->m_index;
		pos.elem->destroy();

		auto next = live_erase(ndx);
		free_push(ndx);

		m_size--;
		return iterator{ data() + next };
	};

	constexpr iterator erase(iterator first, iterator last)
	{
		while (first != last)
		{
			first = erase(first);
		}
		return first;
	};

	// modifiers - push_back
	constexpr void push_back(const T& value)
	{
		if (full())	throw std::bad_alloc();

		auto ndx = free_pop();
		element* it = data() + ndx;
		live_push_back(ndx);
		it->construct(value);
		m_size++;
	};

	constexpr void push_back(T&& value)
	{
		if (full())	throw std::bad_alloc();

		auto ndx = free_pop();
		element* it = data() + ndx;
		live_push_back(ndx);
		it->construct(std::forward<T>(value));
		m_size++;
	};

	// modifiers - emplace_back
	template<class... Args>
	constexpr iterator emplace_back(Args&&... args)
	{
		if (full())	throw std::bad_alloc();

		auto ndx = free_pop();
		element* it = data() + ndx;
		live_push_back(ndx);
		it->construct(std::forward<Args>(args)...);
		m_size++;
		return iterator{ it };
	};

	// modifiers - pop_back
	template<class... Args>
	constexpr void pop_back(Args&&... args)
	{
		if (m_size > 0)
		{
			auto ndx = m_live_tail;
			data()[ndx].destroy();
			live_erase(ndx);
			free_push(ndx);
			m_size--;
		}
	};

	// modifiers - push_front
	constexpr void push_front(const T& value)
	{
		if (full())	throw std::bad_alloc();

		auto ndx = free_pop();
		element* it = data() + ndx;
		live_push_front(ndx);
		it->construct(value);
		m_size++;
	};

	constexpr void push_front(T&& value)
	{
		if (full())	throw std::bad_alloc();

		auto ndx = free_pop();
		element* it = data() + ndx;
		live_push_front(ndx);
		it->construct(std::forward<T>(value));
		m_size++;
	};

	// modifiers - emplace_front
	template<class... Args>
	constexpr reference emplace_front(Args&&... args)
	{
		if (full())	throw std::bad_alloc();

		auto ndx = free_pop();
		element* it = data() + ndx;
		live_push_front(ndx);
		it->construct(std::forward<Args>(args)...);
		m_size++;
		return iterator{ it };
	};

	// modifiers - pop_front
	template<class... Args>
	constexpr void pop_front(Args&&... args)
	{
		if (m_size > 0)
		{
			auto ndx = m_live_head;
			data()[ndx].destroy();
			live_erase(ndx);
			free_push(ndx);
			m_size--;
		}
	};

	// modifiers - swap
	constexpr void swap(slot_array& other) noexcept
	{
		std::swap(m_data, other.m_data);
		std::swap(m_size, other.m_size);
		std::swap(m_live_head, other.m_live_head);
		std::swap(m_live_tail, other.m_live_tail);
		std::swap(m_free_head, other.m_free_head);
	}

	// testing
	size_t free_head() const { return m_free_head; }
	size_t live_head() const { return m_live_head; }
	size_t live_tail() const { return m_live_tail; }

private:

	// element storage
	std::conditional_t<
		!HeapAlloc,
		element[Count + 1],
		std::unique_ptr<element[]>
	> m_data;

	size_type m_size = 0;		// number of objects in container
	size_type m_free_head = 0;	// index of the first free element
	size_type m_live_head = 0;	// index of the first live element 
	size_type m_live_tail = 0;	// index of the last live element  

	void init_storage()
	{
		if constexpr (HeapAlloc)
		{
			m_data = std::make_unique<element[]>(Count + 1);
		}

		// init lists
		init_lists();
	}

	void init_lists()
	{
		auto it = data();
		for (int i = 0; i < capacity(); ++i, ++it)
		{
			it->m_index = i;
			it->m_next = i + 1;
			it->m_prev = i - 1;
		}

		data()[0].m_prev = LIST_HEAD;
		data()[capacity() - 1].m_next = LIST_TAIL;
		m_free_head = 0;
		m_live_head = LIST_TAIL;
		m_live_tail = LIST_TAIL;

		data()[capacity()].m_index = capacity();
		data()[capacity()].m_prev = m_live_tail;
	}

	// free list management
	void free_push(size_type index)
	{
		if (m_free_head != LIST_TAIL) {
			data()[m_free_head].m_prev = index;
		}

		data()[index].m_next = m_free_head;
		data()[index].m_prev = LIST_HEAD;
		m_free_head = index;
	}

	size_type free_pop()
	{
		auto tmp = m_free_head;

		if (data()[tmp].m_next != LIST_TAIL) {
			m_free_head = data()[tmp].m_next;
			data()[m_free_head].m_prev = LIST_HEAD;
		}
		else {
			m_free_head = LIST_TAIL;
		}

		return tmp;
	}

	void free_erase(size_type index)
	{
		element* it = data() + index;
		auto next = it->m_next;
		auto prev = it->m_prev;

		if (m_free_head == index)
		{
			if (prev != LIST_HEAD)
			{
				m_free_head = prev;
			}
			else if (next != LIST_TAIL) {
				m_free_head = next;
			}
			else {
				m_free_head = LIST_TAIL;
			}
		}

		if (next != LIST_TAIL) {
			data()[next].m_prev = prev;
		}

		if (prev != LIST_HEAD) {
			data()[prev].m_next = next;
		}
	}

	// live list management
	void live_push_back(size_type index)
	{

		if (m_live_head == LIST_TAIL)
		{
			m_live_head = index;
		}

		auto& elem = data()[index];
		if (m_live_tail == LIST_TAIL)
		{
			m_live_tail = index;
			elem.m_next = LIST_TAIL;
			elem.m_prev = LIST_HEAD;
			data()[capacity()].m_prev = m_live_tail;
		}
		else
		{
			data()[m_live_tail].m_next = index;

			elem.m_next = LIST_TAIL;
			elem.m_prev = m_live_tail;
			m_live_tail = index;
			data()[capacity()].m_prev = m_live_tail;
		}
	}

	void live_push_front(size_type index)
	{

		if (m_live_tail == LIST_TAIL)
		{
			m_live_tail = index;
			data()[capacity()].m_prev = m_live_tail;
		}

		if (m_live_head != LIST_TAIL)
		{
			data()[m_live_head].m_prev = index;
		}

		data()[index].m_next = m_live_head;
		data()[index].m_prev = LIST_HEAD;
		m_live_head = index;
	}

	// return index of next elem
	size_type live_erase(size_type index)
	{
		element* it = data() + index;
		auto next = it->m_next;
		auto prev = it->m_prev;

		if (m_live_head == index)
		{
			m_live_head = next;
		}

		if (m_live_tail == index)
		{
			m_live_tail = prev;
			data()[capacity()].m_prev = m_live_tail;
		}

		if (next != LIST_TAIL) {
			data()[next].m_prev = prev;
		}

		if (prev != LIST_HEAD) {
			data()[prev].m_next = next;
		}
		return next;
	}

	void live_insert(size_type free_index, size_type before_index)
	{
		if (before_index == m_live_head)
		{
			live_push_front(free_index);
		}
		else if (before_index == LIST_TAIL)
		{
			live_push_back(free_index);
		}
		else
		{
			auto tmp = data()[before_index].m_prev;

			data()[free_index].m_prev = data()[before_index].m_prev;
			data()[free_index].m_next = before_index;
			data()[before_index].m_prev = free_index;
			data()[tmp].m_next = free_index;
		}
	}
};

}
#pragma once

#include <cstdint>
#include <vector>
#include <cassert>
#include <utility>
#include <optional>

namespace ff {

	struct slot_key {
		uint32_t generation = 0;
		uint32_t sparse_index = 0;

		constexpr bool operator==(const slot_key& other) const {
			return other.generation == generation
				&& other.sparse_index == sparse_index;
		}

		constexpr bool operator!=(const slot_key& other) const {
			return other.generation != generation
				|| other.sparse_index != sparse_index;
		}

		constexpr size_t raw() const {
			return { (size_t)generation << 32 | (size_t)sparse_index };
		}
	};

	template<class T>
	class slot_map
	{
	public:
		using size_type = size_t;
		using value_type = T;

		using reference = T&;
		using const_reference = const T&;

		using pointer = T*;
		using const_pointer = const T*;

	private:
		struct sparse_t
		{
			bool valid;			  // there is a T in dense_vector for this key
			uint32_t dense_index; // index to T if valid, or the next empty slot if !valid
			slot_key key;
		};

		using sparse_vector = std::vector<sparse_t>;
		using dense_vector = std::vector<T>;

		// sentinal value for the empty list
		static constexpr size_t EmptyLast = std::numeric_limits<uint32_t>::max();

		static constexpr uint32_t sparse_init = 16; // initial size of sparse when constructed
		static constexpr uint32_t sparse_max = 64; // max size diff of sparse to dense

		// ratio of the size of the dense_vector to sparse_vector
		// used to ensure a minumum number of empty sparse_vector elems
		static constexpr float sparse_density = 0.875f;

	public:
		using iterator = dense_vector::iterator;
		using const_iterator = dense_vector::const_iterator;

		using reverse_iterator = dense_vector::reverse_iterator;
		using const_reverse_iterator = dense_vector::const_reverse_iterator;

	public:
		// constructors
		constexpr slot_map() noexcept
			: sparse_(sparse_init)
		{
			init();
		}

		// accessors
		constexpr const_reference at(const slot_key& k) const {
			assert(k.sparse_index < sparse_.size());
			const sparse_t& sp = sparse_.at((size_t)k.sparse_index);

			if (!sp.valid) {
				throw std::exception{};
			}

			return dense_.at(sp.dense_index);
		}

		constexpr reference at(const slot_key& k) {
			return const_cast<T&>(std::as_const(*this).at(k));
		}

		constexpr const_reference operator[](const slot_key& k) const {
			return at(k);
		}

		constexpr reference operator[](const slot_key& k) {
			return at(k);
		}

		constexpr bool exists(const slot_key& k) const {
			return k.sparse_index < sparse_.size()
				&& sparse_.at(k.sparse_index).valid
				&& sparse_.at(k.sparse_index).key == k;
		}

		// capacity
		constexpr bool empty() const {
			return dense_.empty();
		}

		constexpr size_t size() const {
			return dense_.size();
		}

		// modifiers
		constexpr void clear() {
			dense_.clear();
			init();
		}

		constexpr slot_key push_back(const T& value) {
			return emplace_back(value);
		}

		template<class... Args>
		constexpr slot_key emplace_back(Args&&... args)
		{
			uint32_t sparse_ndx = pop_empty();
			auto& sp = sparse_[(size_t)sparse_ndx];

			sp.valid = true;
			sp.dense_index = (uint32_t)dense_.size();
			sp.key = {
				.generation = sp.key.generation + 1,
				.sparse_index = sparse_ndx
			};

			dense_.emplace_back(std::forward<Args>(args)...);
			dense_to_sparse_.push_back(sparse_ndx);

			while (dense_.size() >= sparse_.size() * sparse_density
				&& sparse_.size() - dense_.size() <= sparse_max)
			{
				push_empty();
			}
			return sp.key;
		}

		constexpr iterator erase(const slot_key& k)
		{
			if (!exists(k))
				return {};

			auto s_ndx = (size_t)k.sparse_index;
			auto d_ndx = sparse_[s_ndx].dense_index;

			auto r = dense_.erase(dense_.begin() + d_ndx);
			auto it = dense_to_sparse_.erase(dense_to_sparse_.begin() + d_ndx);
			for (; it != dense_to_sparse_.end(); it++)
			{
				--sparse_[*it].dense_index;
			}
			sparse_[s_ndx].valid = false;
			push_empty(k.sparse_index);
			return r;
		}

		constexpr iterator erase(const T& value) {
			auto k = key_of(value);
			return k ? erase(*k) : dense_.end();
		}

		constexpr iterator erase(const_iterator it) {
			auto k = key_of(it);
			return k ? erase(*k) : dense_.end();
		}

		// iterators
		constexpr iterator begin() { return dense_.begin(); }
		constexpr const_iterator begin() const { return dense_.begin(); }
		constexpr const_iterator cbegin() const { return dense_.begin(); }

		constexpr iterator end() { return dense_.end(); }
		constexpr const_iterator end() const { return dense_.end(); }
		constexpr const_iterator cend() const { return dense_.end(); }

		constexpr reverse_iterator rbegin() { return dense_.rbegin(); }
		constexpr const_reverse_iterator rbegin() const { return dense_.rbegin(); }
		constexpr const_reverse_iterator crbegin() const { return dense_.rbegin(); }

		constexpr reverse_iterator rend() { return dense_.rend(); }
		constexpr const_reverse_iterator rend() const { return dense_.rend(); }
		constexpr const_reverse_iterator crend() const { return dense_.rend(); }

		// key accessors
		constexpr std::optional<slot_key> key_of(const T& value) const {
			size_t ndx = &value - dense_.data();
			if (ndx < dense_to_sparse_.size())
			{
				return sparse_.at(dense_to_sparse_.at(ndx)).key;
			}
			return {};
		}

		constexpr std::optional<slot_key> key_of(const_iterator it) const {
			return key_of(*it);
		}

	private:
		sparse_vector sparse_;
		dense_vector dense_;
		std::vector<uint32_t> dense_to_sparse_;

		// start of the empty list
		uint32_t first_empty_;

		// last of the empty list
		uint32_t last_empty_;

		// add new empty slots to back of sparse
		void push_empty() {
			sparse_.push_back({});
			sparse_[(size_t)last_empty_].dense_index = (uint32_t)sparse_.size() - 1;
			last_empty_ = (uint32_t)sparse_.size() - 1;
			sparse_.back().valid = false;
			sparse_.back().dense_index = EmptyLast;
		}

		// add exists empty slot
		void push_empty(uint32_t ndx) {
			sparse_[(size_t)last_empty_].dense_index = ndx;
			last_empty_ = ndx;
			sparse_[(size_t)last_empty_].valid = false;
			sparse_[(size_t)last_empty_].dense_index = EmptyLast;
		}

		uint32_t pop_empty() {
			assert(first_empty_ != EmptyLast);
			auto pop = first_empty_;
			first_empty_ = sparse_[(size_t)first_empty_].dense_index;
			return pop;
		}

		void init() {
			assert(dense_.empty());
			for (uint32_t i = 0; i < sparse_.size(); i++)
			{
				sparse_[i].valid = false;
				sparse_[i].dense_index = i + 1;
			}

			first_empty_ = 0;
			last_empty_ = (uint32_t)sparse_.size() - 1;

			sparse_[(size_t)last_empty_].dense_index = EmptyLast;
		}
	};

}


#pragma once

#include <cstdint>
#include <vector>
#include <cassert>
#include <utility>
#include <optional>
#include <limits>

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

        constexpr bool operator<(const slot_key& other) const {
            return other.generation == generation
                ? other.sparse_index < sparse_index
               : other.generation < generation;
        }

        constexpr bool operator<=(const slot_key& other) const {
            return other.generation == generation
                   ? other.sparse_index <= sparse_index
                   : other.generation <= generation;
        }

        constexpr bool operator>(const slot_key& other) const {
            return other.generation == generation
                   ? other.sparse_index > sparse_index
                   : other.generation > generation;
        }

        constexpr bool operator>=(const slot_key& other) const {
            return other.generation == generation
                   ? other.sparse_index >= sparse_index
                   : other.generation >= generation;
        }

		constexpr size_t raw() const {
			return *((size_t*)this);
		}

        constexpr explicit operator bool() const {
            return generation != 0;
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
		using dense_vector = std::vector<std::pair<slot_key, T>>;

		// sentinal value for the empty list
		static constexpr size_t EmptyLast = (std::numeric_limits<uint32_t>::max)();

		static constexpr uint32_t sparse_init = 16; // initial size of sparse when constructed
		static constexpr uint32_t sparse_max = 64; // max size diff of sparse to dense

		// ratio of the size of the dense_vector to sparse_vector
		// used to ensure a minumum number of empty sparse_vector elems
		static constexpr float sparse_density = 0.875f;

	public:
		using iterator               = typename dense_vector::iterator;
		using const_iterator         = typename dense_vector::const_iterator;

		using reverse_iterator       = typename dense_vector::reverse_iterator;
		using const_reverse_iterator = typename dense_vector::const_reverse_iterator;

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

			return dense_.at(sp.dense_index).second;
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

        std::pair<slot_key, T>* data() {
            return dense_.data();
        }
        const std::pair<slot_key, T>* data() const {
            return dense_.data();
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
				.generation = incr_gen(sp.key.generation),
				.sparse_index = sparse_ndx
			};
            auto out_key = sp.key;

			dense_.emplace_back(out_key, value_type(std::forward<Args>(args)...));

			while (dense_.size() >= sparse_.size() * sparse_density
				&& sparse_.size() - dense_.size() <= sparse_max)
			{
				push_empty();
			}
			return out_key;
		}

		constexpr iterator erase(const slot_key& k)
		{
			if (!exists(k))
				return {};

			auto s_ndx = (size_t)k.sparse_index;
			auto d_ndx = sparse_[s_ndx].dense_index;

			auto r = dense_.erase(dense_.begin() + d_ndx);
			sparse_[s_ndx].valid = false;
            // update dense index for sparse entries that pointed past this one
            while (d_ndx < dense_.size())
            {
                sparse_[dense_[d_ndx].first.sparse_index].dense_index = d_ndx;
                ++d_ndx;
            }

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
            if ((&value >= dense_.data()) && (&value < &dense_.back()))
            {
                ptrdiff_t p1 = &dense_.data()->second;
                ptrdiff_t p2 = &value;
                ptrdiff_t diff = (p2 - p1) / sizeof(dense_vector::value_type);
                return { (dense_.data() + diff)->first };
            }
            return{};
		}

		constexpr std::optional<slot_key> key_of(const_iterator it) const {
			return key_of(*it);
		}

        slot_key peek_next_key() const {
            uint32_t sparse_ndx = first_empty_;
            auto& sp = sparse_[(size_t)sparse_ndx];
            return {
                    .generation = incr_gen(sp.key.generation),
                    .sparse_index = sparse_ndx
            };
        }

	private:
		sparse_vector sparse_;
		dense_vector dense_;

		// start of the empty list
		uint32_t first_empty_;

		// last of the empty list
		uint32_t last_empty_;

        static unsigned incr_gen(unsigned gen) { return gen + 1 == 0 ? 1 : gen + 1; }

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


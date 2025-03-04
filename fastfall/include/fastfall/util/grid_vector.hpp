#pragma once

#include <iterator>
#include <algorithm>
#include <stdexcept>

#include <concepts>
#include <optional>

namespace ff {

	namespace detail {
		template<typename V>
		concept vector_type = requires (V v)
		{
			{ v.x } -> std::convertible_to<std::size_t>;
			{ v.y } -> std::convertible_to<std::size_t>;
		};
	}

	template<typename T>
	class grid_vector;

	template<typename T>
	class grid_view;

	template<typename T>
	struct grid_const_iterator;

	template<typename T>
	struct grid_iterator
	{
		using value_type = T;

		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const pointer;

		using size_type = size_t;
		using difference_type = ptrdiff_t;

		using iterator_category = std::bidirectional_iterator_tag;

		constexpr grid_iterator() noexcept
		{
		}

		constexpr grid_iterator& operator=(const grid_iterator& other)
		{
            if (&other != this) {
                m_ptr = other.m_ptr;
                m_curr_column = other.m_curr_column;
                m_curr_row = other.m_curr_row;
                m_columns = other.m_columns;
                m_row_stride = other.m_row_stride;
            }
			return *this;
		}


		constexpr grid_iterator& operator++()
		{
			if (m_curr_column == m_columns - 1)
			{
				m_curr_column = 0;
				m_curr_row++;
				m_ptr += m_row_stride + 1;
			}
			else {
				m_ptr++;
				m_curr_column++;
			}
			return *this;
		}
		constexpr grid_iterator operator++(int)
		{
			auto temp = *this;
			++(*this);
			return temp;
		}

		constexpr grid_iterator& operator--()
		{
			if (m_curr_column == 0)
			{
				m_curr_column = m_columns - 1;
				m_ptr -= m_row_stride + 1;
				m_curr_row--;
			}
			else {
				m_ptr--;
				m_curr_column--;
			}
			return *this;
		}
		constexpr grid_iterator operator--(int)
		{
			auto temp = *this;
			return temp;
		}
		constexpr value_type* operator->() { return m_ptr; }
		constexpr const value_type* operator->() const { return m_ptr; }

		constexpr value_type& operator* () { return *m_ptr; }
		constexpr const value_type& operator* () const { return *m_ptr; }

		constexpr bool operator==(const grid_iterator& other) const noexcept { return m_ptr == other.m_ptr; };
		constexpr bool operator!=(const grid_iterator& other) const noexcept { return m_ptr != other.m_ptr; };

		constexpr bool operator==(const grid_const_iterator<T>& other) const noexcept { return m_ptr == other.m_ptr; };
		constexpr bool operator!=(const grid_const_iterator<T>& other) const noexcept { return m_ptr != other.m_ptr; };

		constexpr size_type column() const { return m_curr_column; };
		constexpr size_type row() const { return m_curr_row; };

	private:

		constexpr grid_iterator(value_type* ptr, size_type column, size_type row, size_type column_count, size_type row_stride = 0) noexcept
			: m_ptr(ptr)
			, m_curr_column(column)
			, m_curr_row(row)
			, m_columns(column_count)
			, m_row_stride(row_stride)
		{
		}

		friend class grid_view<T>;
		friend class grid_vector<T>;

		value_type* m_ptr = nullptr;

		size_type m_curr_column = 0;
		size_type m_curr_row = 0;

		size_type m_columns = 0;
		size_type m_row_stride = 0;
	};

	template<typename T>
	struct grid_const_iterator
	{
		using value_type = T;

		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const pointer;

		using size_type = size_t;
		using difference_type = ptrdiff_t;

		using iterator_category = std::bidirectional_iterator_tag;

		constexpr grid_const_iterator() noexcept
		{
		}

		constexpr grid_const_iterator& operator=(const grid_const_iterator& other)
		{
			m_ptr = other.m_ptr;
			m_curr_column = other.m_curr_column;
			m_curr_row = other.m_curr_row;
			m_columns = other.m_columns;
			m_row_stride = other.m_row_stride;
			return *this;
		}

		constexpr grid_const_iterator& operator++()
		{
			if (m_curr_column == m_columns - 1)
			{
				m_curr_column = 0;
				m_curr_row++;
				m_ptr += m_row_stride + 1;
			}
			else {
				m_ptr++;
				m_curr_column++;
			}
			return *this;
		}
		constexpr grid_const_iterator operator++(int)
		{
			auto temp = *this;
			++(*this);
			return temp;
		}

		constexpr grid_const_iterator& operator--()
		{
			if (m_curr_column == 0)
			{
				m_curr_column = m_columns - 1;
				m_ptr -= m_row_stride + 1;
				m_curr_row--;
			}
			else {
				m_ptr--;
				m_curr_column--;
			}
			return *this;
		}
		constexpr grid_const_iterator operator--(int)
		{
			auto temp = *this;
			return temp;
		}

		constexpr const value_type* operator->() const { return m_ptr; }

		constexpr const value_type& operator* () const { return *m_ptr; }

		constexpr bool operator==(const grid_iterator<T>& other) const noexcept { return m_ptr == other.m_ptr; };
		constexpr bool operator!=(const grid_iterator<T>& other) const noexcept { return m_ptr != other.m_ptr; };

		constexpr bool operator==(const grid_const_iterator& other) const noexcept { return m_ptr == other.m_ptr; };
		constexpr bool operator!=(const grid_const_iterator& other) const noexcept { return m_ptr != other.m_ptr; };

		constexpr size_type column() const noexcept { return m_curr_column; };
		constexpr size_type row() const noexcept { return m_curr_row; };

	private:

		constexpr grid_const_iterator(const value_type* ptr, size_type column, size_type row, size_type column_count, size_type row_stride = 0) noexcept
			: m_ptr(ptr)
			, m_curr_column(column)
			, m_curr_row(row)
			, m_columns(column_count)
			, m_row_stride(row_stride)
		{
		}

		friend class grid_view<T>;
		friend class grid_vector<T>;

		const value_type* m_ptr = nullptr;

		size_type m_curr_column = 0;
		size_type m_curr_row = 0;

		size_type m_columns = 0;
		size_type m_row_stride = 0;
	};


	template <class T>
	class grid_view
	{
	public:
		using value_type = T;

		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const pointer;

		using size_type = size_t;
		using difference_type = ptrdiff_t;

		using iterator = grid_iterator<value_type>;
		using const_iterator = grid_const_iterator<value_type>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;


		constexpr grid_view() noexcept
		{
		}
		constexpr grid_view(const grid_view<T>& x) noexcept
			: m_ptr(x.m_ptr)
			, m_stride(x.m_stride)
			, m_columns(x.m_columns)
			, m_rows(x.m_rows)
		{
		}
		constexpr grid_view(const grid_vector<T>& src) noexcept;

		constexpr grid_view(const grid_view<T>& src, size_type left, size_type top, size_type columns, size_type rows)
			: m_ptr(&src.at(left, top))
			, m_stride((src->column_count() + src.m_stride) - columns)
			, m_columns(columns)
			, m_rows(rows)
		{
			if (left + columns > src.column_size()
				|| top + rows > src.row_size())
			{
				throw std::invalid_argument("invalid grid view");
			}
		}

		template<detail::vector_type Vec>
		constexpr grid_view(const grid_view<T>& src, const Vec& left_top, const Vec& columns_rows)
			: grid_view(src, left_top.x, left_top.y, columns_rows.x, columns_rows.y)
		{
		}

		constexpr grid_view(const grid_vector<T>& src, size_type left, size_type top, size_type columns, size_type rows);

		template<detail::vector_type Vec>
		constexpr grid_view(const grid_vector<T>& src, const Vec& left_top, const Vec& columns_rows)
			: grid_view(src, left_top.x, left_top.y, columns_rows.x, columns_rows.y)
		{
		}

		constexpr grid_view<T>& operator=(const grid_view<T>& x)
		{
			m_ptr = x.m_ptr;
			m_stride = x.m_stride;
			m_columns = x.m_columns;
			m_rows = x.m_rows;
			return *this;
		}

		constexpr const_iterator begin() {
			return const_iterator{ m_ptr, 0, 0, m_columns, m_stride };
		}
		constexpr const_iterator end() {
			return const_iterator{ m_ptr + size() + m_stride * m_rows, 0, m_rows, m_columns, m_stride };
		}
		constexpr const_iterator cbegin() const {
			return const_iterator{ m_ptr, 0, 0, m_columns, m_stride };
		}
		constexpr const_iterator cend() const {
			return const_iterator{ m_ptr + size() + m_stride * m_rows, 0, m_rows, m_columns, m_stride };
		}
		constexpr reverse_iterator rbegin() {
			return reverse_iterator{ m_ptr, 0, 0, m_columns, m_stride };
		}
		constexpr reverse_iterator rend() {
			return reverse_iterator{ m_ptr + size() + m_stride * m_rows, 0, m_rows, m_columns, m_stride };
		}
		constexpr const_reverse_iterator crbegin() const {
			return const_reverse_iterator{ m_ptr, 0, 0, m_columns, m_stride };
		}
		constexpr const_reverse_iterator crend() const {
			return const_reverse_iterator{ m_ptr + size() + m_stride * m_rows, 0, m_rows, m_columns, m_stride };
		}

		constexpr const_reference at(size_type column_x, size_type row_y) const
		{
			return m_ptr[(row_y * (m_columns + m_stride)) + column_x];
		}

		template<detail::vector_type Vec>
		constexpr const_reference at(const Vec& position) const
		{
			return at(position.x, position.y);
		}

		constexpr const_reference operator[] (size_type ndx) const
		{
			auto col = ndx % m_columns;
			auto row = ndx / m_columns;
			return this->at(col, row);
		}

		template<detail::vector_type Vec>
		constexpr const_reference operator[] (const Vec& position) const
		{
			return at(position);
		}

		constexpr grid_view<T> take_view() const
		{
			return grid_view<T>(*this);
		}

		constexpr grid_view<T> take_view(size_type left, size_type top, size_type columns, size_type rows) const
		{
			return grid_view<T>(*this, left, top, columns, rows);
		}

		template<detail::vector_type Vec>
		constexpr grid_view<T> take_view(const Vec& left_top, const Vec& columns_rows) const
		{
			return grid_view<T>(*this, left_top.x, left_top.y, columns_rows.x, columns_rows.y);
		}

		constexpr bool empty() const noexcept { return !m_ptr; }
		constexpr size_type size() const noexcept { return m_rows * m_columns; };

		constexpr value_type* data() noexcept { return m_ptr; }
		constexpr const value_type* data() const noexcept { return m_ptr; }

		constexpr size_type column_count() const noexcept { return m_columns; };
		constexpr size_type row_count() const noexcept { return m_rows; };

		constexpr bool valid(size_type column_x, size_type row_y) const noexcept
		{
			return column_x < column_count()
				&& column_x >= 0
				&& row_y < row_count()
				&& row_y >= 0;
		}

		template<detail::vector_type Vec>
		constexpr bool valid(const Vec& position) const noexcept
		{
			return position.x < column_count()
				&& position.x >= 0
				&& position.y < row_count()
				&& position.y >= 0;
		}

	private:
		const value_type* m_ptr = nullptr;

		size_type m_stride = 0;
		size_type m_columns = 0;
		size_type m_rows = 0;
	};


	template <class T>
	class grid_vector
	{
	public:
		// traits
		using value_type = T;

		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const pointer;

		using size_type = size_t;
		using difference_type = ptrdiff_t;

		using iterator = grid_iterator<value_type>;
		using const_iterator = grid_const_iterator<value_type>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		// constructors

		constexpr grid_vector() noexcept
		{
		};

		constexpr explicit grid_vector(size_type columns, size_type rows)
			: m_columns(columns)
			, m_rows(rows)
		{
			if (size() > 0) {
				m_ptr = new T[m_rows * m_columns]();
			}
			else {
				clear();
			}
		};

		template<detail::vector_type Vec>
		constexpr explicit grid_vector(const Vec& columns_rows)
			: grid_vector(columns_rows.x, columns_rows.y)
		{
		}

		constexpr grid_vector(size_type columns, size_type rows, const value_type& value)
			: m_columns(columns)
			, m_rows(rows)
		{
			if (size() > 0) {
				m_ptr = new T[m_rows * m_columns](value);
				// std::fill_n(m_ptr, size(), value);
			}
			else {
				clear();
			}
		};
		template<detail::vector_type Vec>
		constexpr explicit grid_vector(const Vec& columns_rows, const value_type& value)
			: grid_vector(columns_rows.x, columns_rows.y, value)
		{
		}

		constexpr grid_vector(const grid_vector& x)
			: m_columns(x.m_columns)
			, m_rows(x.m_rows)
		{
			m_ptr = new T[m_rows * m_columns];
			std::copy_n(x.m_ptr, size(), m_ptr);
		};

		constexpr grid_vector(grid_vector&& x) noexcept
		{
			swap(x);
		};

		constexpr grid_vector(grid_view<T> x)
			: m_columns(x.column_count())
			, m_rows(x.row_count())
		{
			m_ptr = new T[m_rows * m_columns];
			std::copy(x.begin(), x.end(), begin());
		};

		constexpr grid_vector(std::initializer_list<std::initializer_list<value_type>> grid_il)
		{
			assign_from_il(grid_il);
		}

		constexpr ~grid_vector()
		{
			if (m_ptr)
				delete[] m_ptr;
		}

		// assignment
		constexpr grid_vector& operator=(const grid_vector& x)
		{
			clear();
			m_rows = x.m_rows;
			m_columns = x.m_columns;
			m_ptr = new T[size()];
			std::copy_n(x.data(), size(), m_ptr);
			return *this;
		}

		constexpr grid_vector& operator=(grid_vector&& x) noexcept
		{
			swap(x);
			return *this;
		}

		constexpr grid_vector& operator=(grid_view<T> x)
		{
			clear();
			m_rows = x.row_size();
			m_columns = x.column_size();
			m_ptr = new T[size()];
			std::copy(x.begin(), x.end(), begin());
			return *this;
		}

		constexpr grid_vector& operator=(std::initializer_list<std::initializer_list<value_type>> grid_il)
		{
			clear();
			assign_from_il(grid_il);
			return *this;
		}

		constexpr iterator begin() { return iterator{ m_ptr, 0, 0, m_columns }; }
		constexpr iterator end() { return iterator{ m_ptr + size(), 0, m_rows, m_columns }; }
		constexpr const_iterator begin() const { return cbegin(); }
		constexpr const_iterator end() const { return cend(); }
		constexpr const_iterator cbegin() const { return const_iterator{ m_ptr, 0, 0, m_columns }; }
		constexpr const_iterator cend() const { return const_iterator{ m_ptr + size(), 0, m_rows, m_columns }; }
		constexpr reverse_iterator rbegin() { return reverse_iterator{ m_ptr, 0, 0, m_columns }; }
		constexpr reverse_iterator rend() { return reverse_iterator{ m_ptr + size(), 0, m_rows, m_columns }; }
		constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator{ m_ptr, 0, 0, m_columns }; }
		constexpr const_reverse_iterator crend() const { return const_reverse_iterator{ m_ptr + size(), 0, m_rows, m_columns }; }

		constexpr grid_view<T> take_view() const
		{
			return grid_view<T>(*this);
		}

		constexpr grid_view<T> take_view(size_type left, size_type top, size_type columns, size_type rows) const
		{
			return grid_view<T>(*this, left, top, columns, rows);
		}

		template<detail::vector_type Vec>
		constexpr grid_view<T> take_view(const Vec& left_top, const Vec& columns_rows) const
		{
			return grid_view<T>(*this, left_top.x, left_top.y, columns_rows.x, columns_rows.y);
		}

		constexpr reference at(size_type column_x, size_type row_y)
		{
			return m_ptr[(row_y * m_columns) + column_x];
		}
		constexpr const_reference at(size_type column_x, size_type row_y) const
		{
			return m_ptr[(row_y * m_columns) + column_x];
		}

		template<detail::vector_type Vec>
		constexpr reference at(const Vec& position)
		{
			return at(position.x, position.y);
		}

		template<detail::vector_type Vec>
		constexpr const_reference at(const Vec& position) const
		{
			return at(position.x, position.y);
		}

		constexpr reference operator[] (size_type ndx)
		{
			return m_ptr[ndx];
		}

		constexpr const_reference operator[] (size_type ndx) const
		{
			return m_ptr[ndx];
		}

		template<detail::vector_type Vec>
		constexpr reference operator[] (const Vec& position)
		{
			return at(position);
		}

		template<detail::vector_type Vec>
		constexpr const_reference operator[] (const Vec& position) const
		{
			return at(position);
		}

		constexpr bool empty() const noexcept { return !m_ptr; }
		constexpr size_type size() const noexcept { return m_rows * m_columns; };

		constexpr value_type* data() noexcept { return m_ptr; }
		constexpr const value_type* data() const noexcept { return m_ptr; }

		constexpr size_type column_count() const noexcept { return m_columns; };
		constexpr size_type row_count() const noexcept { return m_rows; };

		constexpr void clear() noexcept {
			m_columns = 0;
			m_rows = 0;
			if (m_ptr) {
				delete[] m_ptr;
				m_ptr = nullptr;
			}
		}

		constexpr void swap(grid_vector& x)
		{
			std::swap(m_columns, x.m_columns);
			std::swap(m_rows, x.m_rows);
			std::swap(m_ptr, x.m_ptr);
		}

		constexpr bool valid(size_type column_x, size_type row_y) const noexcept
		{
			return column_x < column_count()
				&& column_x >= 0
				&& row_y < row_count()
				&& row_y >= 0;
		}

		template<detail::vector_type Vec>
		constexpr bool valid(const Vec& position) const noexcept
		{
			return position.x < column_count()
				&& position.x >= 0
				&& position.y < row_count()
				&& position.y >= 0;
		}
		
	private:

		constexpr void assign_from_il(std::initializer_list<std::initializer_list<value_type>> grid_il)
		{
			m_rows = grid_il.size();
			size_type row = 0;
			for (auto& row_il : grid_il) {
				if (!m_ptr)
				{
					m_columns = row_il.size();
					m_ptr = new T[m_rows * m_columns];
				}
				else if (row_il.size() != m_columns)
				{
					throw std::invalid_argument("row length mismatch");
				}
				std::copy(row_il.begin(), row_il.end(), m_ptr + (row * m_columns));
				row++;
			}
		}

		size_type m_columns = 0;
		size_type m_rows = 0;
		value_type* m_ptr = nullptr;
	};

	template<typename T>
	constexpr grid_view<T>::grid_view(const grid_vector<T>& src) noexcept
		: m_ptr(src.data())
		, m_stride(0)
		, m_columns(src.column_count())
		, m_rows(src.row_count())
	{
	}

	template<typename T>
	constexpr grid_view<T>::grid_view(const grid_vector<T>& src, size_type left, size_type top, size_type columns, size_type rows)
		: m_ptr(&src.at(left, top))
		, m_stride(src.column_count() - columns)
		, m_columns(columns)
		, m_rows(rows)
	{
		if (left + columns > src.column_count()
			|| top + rows > src.row_count())
		{
			throw std::invalid_argument("invalid grid view");
		}
	}

	/*
	template<typename T>
	constexpr typename grid_view<T>::size_type grid_view<T>::get_stride() const noexcept
	{
		return m_src->column_count() - m_columns;
	}
	*/
}

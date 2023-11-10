#pragma once

#include <iterator>
#include <type_traits>



// A trivial lazily-initialized object wrapper; does not support references
template<typename T>
class lazy
{
public:

    lazy() : initialized_(false) { }
    lazy(const T& x) : initialized_(false) { construct(x); }

    lazy(const lazy& other)
            : initialized_(false)
    {
        if (other.initialized_)
            construct(other.get());
    }

    lazy& operator=(const lazy& other)
    {
        // To the best of my knowledge, there is no clean way around the self
        // assignment check here since T may not be assignable
        if (this != &other)
            construct(other.get());
        return *this;
    }

    ~lazy() { destroy(); }

    void reset() { destroy(); }
    void reset(const T& x) { construct(x); }

    T& get()       { return reinterpret_cast<      T&>(object_); }
    const T& get() const { return reinterpret_cast<const T&>(object_); }

private:

    // Ensure lazy<T> is not instantiated with T as a reference type
    typedef typename std::enable_if<
            !std::is_reference<T>::value
    >::type ensure_t_is_not_a_reference;

    void construct(const T& x)
    {
        destroy();
        new (&object_) T(x);
        initialized_ = true;
    }

    void destroy()
    {
        if (initialized_)
            reinterpret_cast<T&>(object_).~T();
        initialized_ = false;
    }

    typedef typename std::aligned_storage<
            sizeof(T),
            std::alignment_of<T>::value
    >::type storage_type;

    storage_type object_;
    bool initialized_;
};

// An enumerating iterator that transforms an iterator with a value type of T
// into an iterator with a value type of pair<index, T&>.
template <typename IteratorT>
class enumerating_iterator
{
public:

    typedef IteratorT                              inner_iterator;
    typedef std::iterator_traits<IteratorT>        inner_traits;
    typedef typename inner_traits::difference_type inner_difference_type;
    typedef typename inner_traits::reference       inner_reference;

    // A stripped-down version of std::pair to serve as a value type since
    // std::pair does not like having a reference type as a member.
    struct value_type
    {
        value_type(inner_difference_type f, inner_reference s)
                : first(f), second(s) { }

        inner_difference_type first;
        inner_reference       second;
    };

    typedef std::forward_iterator_tag iterator_category;
    typedef inner_difference_type     difference_type;
    typedef value_type&               reference;
    typedef value_type*               pointer;

    explicit enumerating_iterator(inner_iterator it = inner_iterator(),
                                  difference_type index = 0)
            : it_(it), index_(index) { }

    enumerating_iterator& operator++()
    {
        ++index_;
        ++it_;
        return *this;
    }

    enumerating_iterator operator++(int)
    {
        enumerating_iterator old_this(*this);
        ++*this;
        return old_this;
    }

    const value_type& operator*() const
    {
        value_.reset(value_type(index_, *it_));
        return value_.get();
    }

    const value_type* operator->() const { return &**this; }

    friend bool operator==(const enumerating_iterator& lhs,
                           const enumerating_iterator& rhs)
    {
        return lhs.it_ == rhs.it_;
    }

    friend bool operator!=(const enumerating_iterator& lhs,
                           const enumerating_iterator& rhs)
    {
        return !(lhs == rhs);
    }

private:

    // Ensure that the template argument passed to IteratorT is a forward
    // iterator; if template instantiation fails on this line, IteratorT is
    // not a valid forward iterator:
    typedef typename std::enable_if<
            std::is_base_of<
                    std::forward_iterator_tag,
                    typename std::iterator_traits<IteratorT>::iterator_category
            >::value
    >::type ensure_iterator_t_is_a_forward_iterator;

    inner_iterator it_;              //< The current iterator
    difference_type index_;          //< The index at the current iterator
    mutable lazy<value_type> value_; //< Pair to return from op* and op->
};

// enumerating_iterator<T> construction type deduction helpers
template <typename IteratorT>
enumerating_iterator<IteratorT> make_enumerator(IteratorT it)
{
    return enumerating_iterator<IteratorT>(it);
};

template <typename IteratorT, typename DifferenceT>
enumerating_iterator<IteratorT> make_enumerator(IteratorT it, DifferenceT idx)
{
    return enumerating_iterator<IteratorT>(it, idx);
};
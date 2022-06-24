#pragma once

#include <optional>
#include <vector>
#include <new>

template<class T>
class slot_vector
{
public:
    struct key {
        uint32_t index;
        uint32_t generation;

        size_t raw() const {
            return *std::launder(reinterpret_cast<size_t*>(this));
        }
    };

    struct slot
    {
        slot() = default;
        slot(uint32_t ndx)
            : index(ndx)
        {
        }

        using value_type = T;

        operator bool() const {
            return value.has_value();
        }

        bool has_value() const {
            return value.has_value();
        }

        key key() const {
            return { .index = index, .generation = generation };
        }

        T* operator-> () { return &*value; }
        const T* operator-> () const { return &*value; }

        T& operator* () { return *value; }
        const T& operator* () const { return *value; }

        T& get() {
            return *value;
        }

        template<class... Args>
        T& emplace(Args&&... args)
        {
            if (value) {
                ++generation;
            }
            value.emplace(std::forward<T>(args)...);
            return *value;
        }

    private:

        void erase() {
            if (value) {
                value.reset();
                ++generation;
            }
        }

        uint32_t index;
        uint32_t generation = 0;
        std::optional<T> value = std::nullopt;

        friend class slot_vector<T>;
    };

    using iterator = std::vector<slot>::iterator;
    using const_iterator = std::vector<slot>::const_iterator;
    using reverse_iterator = std::vector<slot>::reverse_iterator;
    using const_reverse_iterator = std::vector<slot>::const_reverse_iterator;

public:
    constexpr slot_vector() = default;

    constexpr slot_vector(size_t init_reseve)
    {
        m_slots.resize(init_reseve);
        size_t n = 0;
        for (auto& slot : m_slots)
        {
            slot.index = n; ++n;
        }
        empty_slots = init_reseve;
    }

    void reserve(size_t size)
    {
        size_t prev_size = m_slots.size();
        if (prev_size < size)
        {
            empty_slots += size - prev_size;
            m_slots.resize(size);
            for (auto i = prev_size; i < m_slots.size(); i++)
            {
                m_slots[i].index = i;
            }
        }
    }

    constexpr slot_vector(const slot_vector& other) = default;
    constexpr slot_vector(slot_vector&& other) noexcept = default;

    slot_vector& operator=(const slot_vector& other) = default;
    slot_vector& operator=(slot_vector&& other) noexcept = default;

    void swap(slot_vector& other) noexcept 
    {
        std::swap(m_slots, other.m_slots);
        std::swap(empty_slots, other.empty_slots);
    }

    bool exists(key key) const
    {
        auto& slot = m_slots.at(key.index);
        return slot.value && slot.generation == key.generation;
    }

    const T& operator[] (key key)
    {
        auto& slot = m_slots.at(key.index);
        if (slot.value && slot.generation == key.generation)
        {
            return *slot.value;
        }
        throw std::exception{};
    }

    const T& operator[] (key key) const
    {
        auto& slot = m_slots.at(key.index);
        if (slot.value && slot.generation == key.generation)
        {
            return *slot.value;
        }
        throw std::exception{};
    }

    T& at(key key)
    {
        auto& slot = m_slots.at(key.index);
        if (slot.value && slot.generation == key.generation)
        {
            return *slot.value;
        }
        throw std::exception{};
    }

    template<class... Args>
    std::pair<key, T*> emplace(Args&&... args)
    {
        auto& slot = get_empty();
        --empty_slots;
        slot.emplace(std::forward<Args>(args)...);
        return { slot.key(), &*slot.value };
    }


    std::pair<key, T*> insert(const T& value)
    {
        auto& slot = get_empty();
        slot.value = value;
        --empty_slots;
        return { slot.key(), &*slot.value };
    }

    bool erase(key key) {
        auto& slot = m_slots.at(key.index);
        if (slot.value && slot.generation == key.generation) {
            slot.erase();
            ++empty_slots;
            return true;
        }
        return false;
    }

    bool erase(const_iterator it) {
        return erase(it->key());
    }

    template<class... Args>
    std::pair<key, T*> emplace_back(Args&&... args)
    {
        push_empty();
        m_slots.back().emplace(std::forward<Args>(args)...);
        return { m_slots.back().key(), &*m_slots.back().value };
    }

    std::pair<key, T*> push_back(const T& value)
    {
        push_empty();
        m_slots.back().value = value;
        return { m_slots.back().key(), &*m_slots.back().value };
    }

    void clear() {
        for (auto& slot : m_slots)
        {
            slot.erase();
        }
        empty_slots = m_slots.size();
    }

    bool empty() const {
        return m_slots.size() == empty_slots;
    }

    size_t size() const {
        return m_slots.size() - empty_slots;
    }

    size_t capacity() const {
        return m_slots.size();
    }

    iterator begin() { return m_slots.begin(); };
    const_iterator begin() const { return m_slots.begin(); };
    const_iterator cbegin() const { return m_slots.begin(); };

    iterator end() { return m_slots.end(); };
    const_iterator end() const { return m_slots.end(); };
    const_iterator cend() const { return m_slots.end(); };

    iterator rbegin() { return m_slots.rbegin(); };
    const_iterator rbegin() const { return m_slots.rbegin(); };
    const_iterator crbegin() const { return m_slots.rbegin(); };

    iterator rend() { return m_slots.rend(); };
    const_iterator rend() const { return m_slots.rend(); };
    const_iterator cernd() const { return m_slots.rend(); };

private:

    void push_empty() {
        m_slots.push_back(slot{ (uint32_t)m_slots.size() });
        ++empty_slots;
    }

    slot& get_empty() {
        if (empty_slots == 0)
        {
            push_empty();
            return m_slots.back();
        }
        else {
            auto it = std::find_if(begin(), end(),
                [](const slot& a_slot) {
                    return !a_slot.has_value();
                });
            return *it;
        }
    }

    std::vector<slot> m_slots;
    size_t empty_slots = 0;
};

#pragma once

#include "fastfall/util/id.hpp"
#include "fastfall/game_v2/World.hpp"

namespace ff {

template<class T>
class id_ptr {
public:
    id_ptr() = default;
    id_ptr(World* w, std::optional<ID<T>> t_id = std::nullopt)
        : m_world(w)
        , m_id(t_id)
    {
    }

    id_ptr(const id_ptr<T>& other) {
        m_world = other.m_world;
        if (other.m_id) {
            m_id = m_world->clone(other.m_id);
        }
    }

    id_ptr(id_ptr<T>&& other) {
        swap(other);
        other.reset();
        return *this;
    }

    id_ptr<T>& operator=(const id_ptr<T>& other) {
        m_world = other.m_world;
        if (other.m_id) {
            m_id = m_world->clone(other.m_id);
        }
    }

    id_ptr<T>& operator=(id_ptr<T>&& other) {
        swap(other);
        other.reset();
        return *this;
    }

    ~id_ptr() {
        if (m_world && m_id) {
            m_world->erase(*m_id);
        }
    }

    void set_world(World* w) { m_world = w; }
    World* world() const { return m_world; }

    std::optional<ID<T>> id() const { return m_id; }

    T* get() const {
        return m_world->get(*m_id);
    }

    ID<T> release() {
        auto _id = m_id;
        m_id.reset();
        return _id;
    }

    void reset(std::optional<ID<T>> n_id = std::nullopt) {
        auto _id = m_id;
        if (_id && _id.value() != n_id) {
            m_world->erase(*_id);
        }
        _id = n_id;
    }

    void swap(id_ptr<T>& other) noexcept {
        std::swap(m_world, other.m_world);
        std::swap(m_id, other.m_id);
    }

    explicit operator bool() const noexcept {
        return m_world && m_id && m_world->get(*m_id);
    }

    T& operator*() const {
        return m_world->at(*m_id);
    }

    T* operator->() const {
        return m_world->get(*m_id);
    }

private:
    std::optional<ID<T>> m_id;
    World* m_world = nullptr;
};

}
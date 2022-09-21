#pragma once

#include "fastfall/util/id.hpp"

#include <optional>

namespace ff {

class World;

template<class T>
class unique_id {
private:
    World* world = nullptr;
    ID<T> id;

public:
    unique_id() = default;
    unique_id(World* t_world, ID<T> t_id)
    : world(t_world)
    , id(t_id)
    {
        assert(t_world);
    }

    unique_id(const unique_id&) = delete;
    unique_id& operator=(const unique_id&) = delete;

    unique_id(unique_id&&) noexcept = default;
    unique_id& operator=(unique_id&&) noexcept = default;

    ~unique_id() {
        reset();
    }

    T* operator->() {
        return world->get(id);
    }

    T& operator*() {
        return world->at(id);
    }

    void update_world(World* w) {
        assert(w);
        world = w;
    }

    void reset() {
        if (world)
            world->erase(id);
    }
};

/*
template<class T>
class uniq_id_ptr {
private:
    struct data_t {
        World* world;
        ID<T> id;
    };
    std::optional<data_t> data = std::nullopt;

public:
    uniq_id_ptr() = default;
    uniq_id_ptr(World* w, ID<T> id)
        : data{data_t{w, id}}
    {
    }

    uniq_id_ptr(const uniq_id_ptr<T>&) = default;
    uniq_id_ptr<T>& operator=(const uniq_id_ptr<T>&) = default;

    uniq_id_ptr(uniq_id_ptr<T>&& other) noexcept {
        swap(other);
    }

    uniq_id_ptr<T>& operator=(uniq_id_ptr<T>&& other) noexcept {
        swap(other);
        other.reset();
        return *this;
    }

    ~uniq_id_ptr() {
        reset();
    }

    std::optional<ID<T>> id() const { return data ? data->id : std::nullopt; }

    void set_world(World& w) { if (data) { data->world = &w; } }

    [[nodiscard]]
    World* get_world() const { return data ? data->world : nullptr; }

    T* get() const {
        return data ? data->world->get(data->id) : nullptr;
    }

    std::optional<ID<T>> release() {
        return data ? data->id : std::nullopt;
    }

    void reset() {
        if (data) {
            data->world->erase(data->id);
        }
    }

    void reset(World& w, ID<T> id) {
        reset();
        data = data_t{ &w, id };
    }

    void swap(uniq_id_ptr<T>& other) noexcept {
        std::swap(data, other.data);
    }

    explicit operator bool() const noexcept {
        return data.has_value();
    }

    T& operator*() const {
        return *get();
    }

    T* operator->() const {
        return get();
    }
};
*/

}
#pragma once

#include "Vec2.hpp"

namespace ff {

template<class T>
struct InterpVec2
{
    InterpVec2() = default;
    InterpVec2(Vec2<T> p) { reset(p); };

    Vec2<T>& operator*() { return _curr; }
    const Vec2<T>& operator*() const { return _curr; }

    Vec2<T>* operator->() { return &_curr; }
    const Vec2<T>* operator->() const { return &_curr; }

    Vec2<T>& get() { return _curr; }
    const Vec2<T>& get() const { return _curr; }

    void reset(Vec2<T> p) { _prev = p; _curr = p; }

    Vec2<T> interpolate(float t) const { return _prev + (_curr - _prev) * t; }

    Vec2<T>& get_prev() { return _prev; }
    const Vec2<T>& get_prev() const { return _prev; }

protected:
    Vec2<T> _curr;
    Vec2<T> _prev;
};

struct Position : public InterpVec2<float> {
public:
    using InterpVec2<float>::InterpVec2;
    void position_advance() { _prev = _curr; }
};

struct Velocity : public InterpVec2<float> {
public:
    using InterpVec2<float>::InterpVec2;
    void velocity_advance() { _prev = _curr; }
};




}
#pragma once

#include "Vec2.hpp"
#include "Rect.hpp"

namespace ff {

/*
template<class T>
struct InterpVec2
{
    InterpVec2() = default;
    InterpVec2(Vec2<T> curr) { reset(curr); };
    InterpVec2(Vec2<T> curr, Vec2<T> prev) { reset(curr, prev); };

    Vec2<T>& operator*() { return _curr; }
    const Vec2<T>& operator*() const { return _curr; }

    Vec2<T>* operator->() { return &_curr; }
    const Vec2<T>* operator->() const { return &_curr; }

    Vec2<T>& get() { return _curr; }
    const Vec2<T>& get() const { return _curr; }

    void reset(Vec2<T> curr) { _prev = curr; _curr = curr; }
    void reset(Vec2<T> curr, Vec2<T> prev) { _prev = prev; _curr = curr; }

    Vec2<T> interpolate(float t) const { return _prev + (_curr - _prev) * t; }

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

template<class T>
struct InterpRect {
    InterpRect() = default;
    InterpRect(Rect<T> curr) { reset(curr); };
    InterpRect(Rect<T> curr, Rect<T> prev) { reset(curr, prev); };

    Rect<T>& operator*() { return _curr; }
    const Rect<T>& operator*() const { return _curr; }

    Rect<T>* operator->() { return &_curr; }
    const Rect<T>* operator->() const { return &_curr; }

    Rect<T>& get() { return _curr; }
    const Rect<T>& get() const { return _curr; }

    void reset(Rect<T> curr) { _prev = curr; _curr = curr; }
    void reset(Rect<T> curr, Rect<T> prev) { _prev = prev; _curr = curr; }

    Rect<T> interpolate(float t) const {




    }


    const Rect<T>& get_prev() const { return _prev; }

protected:
    Rect<T> _curr;
    Rect<T> _prev;
};

 */

}
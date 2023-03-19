#pragma once

#include "Vec2.hpp"

#include <cmath>

namespace ff {

/*
template<typename T>
struct Position {
	Vec2<T> previous;
	Vec2<T> current;

	Position()
	{
	}

	Position(Vec2<T> init_pos)
		: previous(init_pos)
		, current(init_pos)
	{
	}

	Position(Vec2<T> init_prev, Vec2<T> init_curr)
		: previous(init_prev)
		, current(init_curr)
	{
	}

	Vec2<T> get(float interpolation = 1.f) const noexcept {
		if (interpolation == 1.f) {
			return current;
		}
		else if (interpolation == 0.f) {
			return previous;
		}
		else {
			return Vec2<T>{
				std::lerp(previous.x, current.x, interpolation),
				std::lerp(previous.y, current.y, interpolation)
			};
		}
	}

	void set_current(const Vec2<T>& to) noexcept {
		current = to;
	}
	void set_previous(const Vec2<T>& to) noexcept {
		previous = to;
	}

	Vec2<T> get_current() const noexcept {
		return current;
	}
	Vec2<T> get_previous() const noexcept {
		return previous;
	}

	void move_to(const Vec2<T>& to) noexcept {
		previous = current;
		current = to;
	}

	void reset(const Vec2<T>& to) noexcept {
		previous = to;
		current = to;
	}
};

using Positionf = Position<float>;

*/

}
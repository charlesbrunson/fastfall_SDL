#pragma once

#include "fastfall/util/math.hpp"

namespace ff {

class ColliderSurface {
public:
	/*
	constexpr ColliderSurface() :
		surface(),
		ghostp0(),
		ghostp3()
	{

	};
	constexpr ColliderSurface(Linef colSurface, Vec2f ghost0 = Vec2f(), Vec2f ghost3 = Vec2f()) noexcept :
		surface(colSurface),
		ghostp0(ghost0),
		ghostp3(ghost3)
	{

	};

	~ColliderSurface() = default;
	*/


	// ghost point provide additional info for collision 
	// (ghostp0 -> surface.p1 -> surface.p2 -> ghost3)

	Linef surface;
	Vec2f ghostp0;
	Vec2f ghostp3;

	bool g0virtual = true;
	bool g3virtual = true;

	const ColliderSurface* prev = nullptr;
	const ColliderSurface* next = nullptr;

	//additional properties?
};

}
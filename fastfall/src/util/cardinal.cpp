#include "fastfall/util/cardinal.hpp"

namespace ff {

Cardinal cardinalOpposite(Cardinal direction) noexcept {
	Cardinal r = Cardinal::NORTH;
	switch (direction) {
	case Cardinal::NORTH:	r = Cardinal::SOUTH; break;
	case Cardinal::EAST:	r = Cardinal::WEST;  break;
	case Cardinal::SOUTH:	r = Cardinal::NORTH; break;
	case Cardinal::WEST:	r = Cardinal::EAST;  break;
	default: break;
	}
	return r;
}

std::vector<Cardinal> cardinalFromBits(unsigned cardinalBits) noexcept {
	std::vector<Cardinal> r;
	r.reserve(4);
	if (cardinalBits & cardinalBit[Cardinal::NORTH]) r.push_back(Cardinal::NORTH);
	if (cardinalBits & cardinalBit[Cardinal::EAST]) r.push_back(Cardinal::EAST);
	if (cardinalBits & cardinalBit[Cardinal::SOUTH]) r.push_back(Cardinal::SOUTH);
	if (cardinalBits & cardinalBit[Cardinal::WEST]) r.push_back(Cardinal::WEST);
	return r;
};

std::string cardinalToString(Cardinal direction) noexcept {
	switch (direction) {
	case Cardinal::NORTH: return "North";
	case Cardinal::EAST:  return "East";
	case Cardinal::SOUTH: return "South";
	case Cardinal::WEST:  return "West";
	};
	return "Not a valid direction";
}

}
#pragma once

struct QuadID
{
	int value = -1;

	operator bool() const { return value >= 0; }
	bool valid() const { return value >= 0; }

	bool operator<(const QuadID& rhs) const { return value < rhs.value; };
	bool operator==(const QuadID& rhs) const { return value == rhs.value; };
};
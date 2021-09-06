#pragma once

#include <type_traits>

namespace ff {

template<typename T>
requires std::is_copy_assignable_v<T>
class Default {
public:
	Default(T init_value) 
		: def_value(init_value)
		, mod_value(init_value)
	{
	}

	void restore() { mod_value = def_value; }

	operator T() { return mod_value; }

	T& get() { return mod_value; }
	const T& get() const { return mod_value; }

protected:
	const T def_value;
	T mod_value;

};

}
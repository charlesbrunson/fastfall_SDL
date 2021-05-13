#pragma once

#include <SDL_joystick.h>

class Gamepad {
public:
	Gamepad(int device) {
		handle = SDL_JoystickOpen(device);
	}
	~Gamepad() {
		if (handle && SDL_JoystickGetAttached(handle)) 
		{
			SDL_JoystickClose(handle);
		}
	}

	Gamepad(const Gamepad&) = delete;
	Gamepad* operator=(const Gamepad&) = delete;


	Gamepad(Gamepad&& pad) noexcept
	{
		std::swap(handle, pad.handle);
	}
	Gamepad& operator=(Gamepad&& pad) noexcept
	{
		std::swap(handle, pad.handle);
		return *this;
	}

	inline bool isConnected() {
		return handle && SDL_JoystickGetAttached(handle);
	}

	inline SDL_Joystick* getHandle() {
		return handle;
	}

protected:
	SDL_Joystick* handle = nullptr;
};
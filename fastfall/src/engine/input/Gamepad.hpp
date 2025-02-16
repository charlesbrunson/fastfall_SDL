#pragma once

#include "SDL3/SDL_joystick.h"

class Gamepad {
public:
	Gamepad(int device) {
		handle = SDL_OpenGamepad(device);
	}
	~Gamepad() {
		if (handle && SDL_GamepadConnected(handle))
		{
			SDL_CloseGamepad(handle);
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
		return handle && SDL_GamepadConnected(handle);
	}

	inline SDL_Gamepad* getHandle() {
		return handle;
	}

protected:
	SDL_Gamepad* handle = nullptr;
};
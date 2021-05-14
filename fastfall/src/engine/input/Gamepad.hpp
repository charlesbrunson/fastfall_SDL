#pragma once

#include <SDL_joystick.h>

class Gamepad {
public:
	Gamepad(int device) {
		handle = SDL_GameControllerOpen(device);
	}
	~Gamepad() {
		if (handle && SDL_GameControllerGetAttached(handle)) 
		{
			SDL_GameControllerClose(handle);
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
		return handle && SDL_GameControllerGetAttached(handle);
	}

	inline SDL_GameController* getHandle() {
		return handle;
	}

protected:
	SDL_GameController* handle = nullptr;
};
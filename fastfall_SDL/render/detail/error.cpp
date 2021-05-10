
#include "error.hpp"
#include "SDL.h"

namespace ff {

    Error::Error() : Error(SDL_GetError()) {}

    Error::Error(const std::string& message)
        : std::runtime_error(std::string("SDL error: ") + message)
    {

    }

}
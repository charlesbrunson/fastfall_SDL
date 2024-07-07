#pragma once

#include <SDL2/SDL.h>
#include "ff/util/log.hpp"

#if defined(DEBUG) && not defined(__EMSCRIPTEN__)

namespace ff {
    inline void checkSDL(int result) {
        if (result < 0) {
            std::string err = SDL_GetError();
            ff::error("SDL: {}", err);
            throw std::runtime_error(err);
        }
    }

    template <typename ObjectPtr>
    auto checkSDL(ObjectPtr pObject) {
        if (pObject == nullptr) {
            std::string err = SDL_GetError();
            ff::error("SDL: {}", err);
            throw std::runtime_error(err);
        }
        return pObject;
    }
}

#else
#define checkSDL(expr) (expr)
#endif

namespace ff {
    bool sdl_init();
    void sdl_quit();
}

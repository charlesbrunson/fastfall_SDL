#include "ff/engine.hpp"

#include "ff/util/log.hpp"

#include "external/SDL.hpp"
#include <SDL_image.h>

#include "external/glew.hpp"
#include "external/freetype.hpp"

namespace ff {

bool initialize() {
    checkSDL(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO));

    SDL_version version;
    SDL_GetVersion(&version);
    info("{:>10} {}.{}.{}", "SDL", version.major, version.minor, version.patch);

    checkSDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1));
    //checkSDL(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1));

    checkSDL(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0));

#if defined(__EMSCRIPTEN__)
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES));
#else
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
#endif

    int flags = IMG_INIT_PNG;
    int outflags = IMG_Init(flags);
    if (outflags != flags) {
        ff::error("IMG init failed: {}", IMG_GetError());
        return false;
    }

    freetype_init();
    return true;
}

bool shutdown() {
    return true;
}

}
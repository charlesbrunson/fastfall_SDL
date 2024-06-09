#pragma once

#include "GL/glew.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

#if defined(DEBUG) && not defined(__EMSCRIPTEN__)
void glCheckError(const char* file, unsigned int line, const char* expression);
#define glCheck(expr) do { expr; glCheckError(__FILE__, __LINE__, #expr); } while (false)
#else
#define glCheck(expr) (expr)
#endif

namespace ff {

bool glew_init();

}
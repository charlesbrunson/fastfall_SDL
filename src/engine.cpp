#include "ff/engine.hpp"

#include "ff/util/log.hpp"

#include <SDL2/SDL.h>
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
    }

    freetype_init();

    GLenum glew_err = glewInit();
    if (GLEW_OK != glew_err) {
        std::string err = (char *)glewGetErrorString(glew_err);
        ff::error("Unable to init glew: {}", err);
        return false;
    }
    ff::info("{:>10} {}", "GLEW", (const char*)glewGetString(GLEW_VERSION));


    GLint glvmajor, glvminor;
    glGetIntegerv(GL_MAJOR_VERSION, &glvmajor);
    glGetIntegerv(GL_MINOR_VERSION, &glvminor);
#if defined(__EMSCRIPTEN__)
    ff::info("{:>10} {}.{}", "OpenGL ES", glvmajor, glvminor);
#else
    ff::info("{:>10} {}.{}", "OpenGL", glvmajor, glvminor);
#endif

    ff::info("OpenGL Vendor:         {}", (const char*)glGetString(GL_VENDOR));
    ff::info("OpenGL Renderer:       {}", (const char*)glGetString(GL_RENDERER));
    ff::info("OpenGL Version:        {}", (const char*)glGetString(GL_VERSION));
    ff::info("OpenGL Shader Version: {}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    //ShaderProgram::getDefaultProgram();
    //ff::info("Loaded default shader");

#if not defined(__EMSCRIPTEN__)
    glCheck(glEnable(GL_DEBUG_OUTPUT));
    glDebugMessageCallback(MessageCallback, 0);
#endif

    glCheck(glDisable(GL_CULL_FACE));
    glCheck(glDisable(GL_DEPTH_TEST));
    glCheck(glEnable(GL_BLEND));
}

bool shutdown() {

}

}
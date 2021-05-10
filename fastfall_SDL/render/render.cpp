
#include "render.hpp"

#include <stdexcept>
#include <string>

#include "detail/error.hpp"

#include <SDL.h>
#include <gl\glew.h>
#include <SDL_opengl.h>
#include <gl\glu.h>

#include "SDL_image.h"

#include <iostream>

namespace ff {
namespace {
    bool renderInitialized = false;
    bool glewInitialized   = false;


    void GLAPIENTRY
        MessageCallback(GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar* message,
            const void* userParam)
    {
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
    }

}

bool init()
{
    assert(!renderInitialized);
    renderInitialized = true;

    checkSDL(SDL_Init(SDL_INIT_VIDEO));

    checkSDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1));

    checkSDL(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8));

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));

    int flags = IMG_INIT_PNG;
    int outflags = IMG_Init(flags);
    if (outflags != flags) {
        std::cout << IMG_GetError();
        renderInitialized = false;
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    return renderInitialized;
}

void quit()
{
    assert(renderInitialized);
    IMG_Quit();
    SDL_Quit();
    renderInitialized = false;
}

bool isInit()
{
	return renderInitialized;
}

void initGLEW() {
    if (glewInitialized) return;

    GLenum glew_err = glewInit();
    if (GLEW_OK != glew_err) {
        throw std::string("Unable to init glew: ") + (char*)glewGetErrorString(glew_err);
    }
    glewInitialized = true;

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
}

}


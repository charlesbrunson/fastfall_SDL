#include "glew.hpp"

#include <string_view>

#include "ff/util/log.hpp"

#if defined(DEBUG) && not defined(__EMSCRIPTEN__)
void glCheckError(const char* file, unsigned int line, const char* expression) {
    // Get the last error

    GLenum errorCode;
    if ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string_view fileString = file;
        std::string_view error = "Unknown error";
        std::string_view description = "No description";

        // Decode the error code
        switch (errorCode)
        {
            case GL_INVALID_ENUM:
            {
                error = "GL_INVALID_ENUM";
                description = "An unacceptable value has been specified for an enumerated argument.";
                break;
            }
            case GL_INVALID_VALUE:
            {
                error = "GL_INVALID_VALUE";
                description = "A numeric argument is out of range.";
                break;
            }
            case GL_INVALID_OPERATION:
            {
                error = "GL_INVALID_OPERATION";
                description = "The specified operation is not allowed in the current state.";
                break;
            }
            case GL_STACK_OVERFLOW:
            {
                error = "GL_STACK_OVERFLOW";
                description = "This command would cause a stack overflow.";
                break;
            }
            case GL_STACK_UNDERFLOW:
            {
                error = "GL_STACK_UNDERFLOW";
                description = "This command would cause a stack underflow.";
                break;
            }
            case GL_OUT_OF_MEMORY:
            {
                error = "GL_OUT_OF_MEMORY";
                description = "There is not enough memory left to execute the command.";
                break;
            }
            case GL_INVALID_FRAMEBUFFER_OPERATION:
            {
                error = "GL_INVALID_FRAMEBUFFER_OPERATION";
                description = "The object bound to FRAMEBUFFER_BINDING is not \"framebuffer complete\".";
                break;
            }
        }

        ff::error("");
        ff::error("An internal OpenGL call failed in");
        ff::error("{}({})", fileString.substr(fileString.find_last_of("\\/") + 1), line);
        ff::error("Expression:");
        ff::error("\t{}", expression);
        ff::error("Error description:");
        ff::error("\t{}", error);
        ff::error("\t{}", description);

    }
}
#endif

void GLAPIENTRY
MessageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar *message,
                const void *userParam) {

    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
                (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
                type, severity, message);
    }
}

namespace ff {

bool glew_is_init = false;

bool glew_init() {
    if (glew_is_init) {
        return glew_is_init;
    }

    GLenum glew_err = glewInit();
    if (GLEW_OK != glew_err) {
        std::string_view err = (char*)glewGetErrorString(glew_err);
        ff::error("Unable to init glew: {}", err);
        return glew_is_init;
    }
    ff::info("{}: {}", "GLEW", (const char*)glewGetString(GLEW_VERSION));

    GLint glvmajor, glvminor;
    glGetIntegerv(GL_MAJOR_VERSION, &glvmajor);
    glGetIntegerv(GL_MINOR_VERSION, &glvminor);
#if defined(__EMSCRIPTEN__)
    ff::info("{:>10} {}.{}", "OpenGL ES", glvmajor, glvminor);
#else
    ff::info("{}: {}.{}", "OpenGL", glvmajor, glvminor);
#endif

    ff::info("OpenGL Vendor:  {}", (const char*)glGetString(GL_VENDOR));
    ff::info("OpenGL Renderer: {}", (const char*)glGetString(GL_RENDERER));
    ff::info("OpenGL Version: {}", (const char*)glGetString(GL_VERSION));
    ff::info("OpenGL Shader Version: {}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

#if not defined(__EMSCRIPTEN__)
    glCheck(glEnable(GL_DEBUG_OUTPUT));
    glDebugMessageCallback(MessageCallback, 0);
#endif

    glCheck(glDisable(GL_CULL_FACE));
    glCheck(glDisable(GL_DEPTH_TEST));
    glCheck(glEnable(GL_BLEND));

    glew_is_init = true;
    return glew_is_init;
}

}
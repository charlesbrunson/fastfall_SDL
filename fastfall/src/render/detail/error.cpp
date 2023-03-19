
#include "error.hpp"

#include "fastfall/render/external/opengl.hpp"
#include "fastfall/util//log.hpp"

namespace ff {

    Error::Error() : Error(SDL_GetError()) {}

    Error::Error(const std::string& message)
        : std::runtime_error(std::string("SDL error: ") + message)
    {

    }


    void glCheckError(const char* file, unsigned int line, const char* expression) {

#if not defined(__EMSCRIPTEN__)

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

            LOG_ERR_("");
            LOG_ERR_("An internal OpenGL call failed in");
            LOG_ERR_("{}({})", fileString.substr(fileString.find_last_of("\\/") + 1), line);
            LOG_ERR_("Expression:");
            LOG_ERR_("\t{}", expression);
            LOG_ERR_("Error description:");
            LOG_ERR_("\t{}", error);
            LOG_ERR_("\t{}", description);

        }
#endif
    }
}

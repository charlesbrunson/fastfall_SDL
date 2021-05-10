#pragma once

#include <stdexcept>
#include <string>

namespace ff {
    class Error : public std::runtime_error {
    public:
        Error();
        explicit Error(const std::string& message);
    };

    inline void checkSDL(int result) {
        if (result < 0) {
            throw Error();
        }
    }

    template <typename ObjectPtr>
    auto checkSDL(ObjectPtr pObject) {
        if (pObject == nullptr) {
            throw Error();
        }
        return pObject;
    }

    void glCheckError(const char* file, unsigned int line, const char* expression);

}


#ifdef DEBUG

// In debug mode, perform a test on every OpenGL call
// The do-while loop is needed so that glCheck can be used as a single statement in if/else branches
#define glCheck(expr) do { expr; glCheckError(__FILE__, __LINE__, #expr); } while (false)

#else

// Else, we don't add any overhead
#define glCheck(expr) (expr)

#endif
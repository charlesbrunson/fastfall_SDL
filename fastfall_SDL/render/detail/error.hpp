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

}
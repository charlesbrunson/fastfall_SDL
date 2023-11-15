#pragma once

#include "fastfall/util/math.hpp"

namespace ff {
    template<typename T>
    class PlotLine {
        class iterator {
        public:
            iterator();
            Vec2<int> operator*() const;

            bool operator==(const iterator& rhs) const;
            bool operator!=(const iterator& rhs) const;
            void operator++();
        };

    public:
        PlotLine(Line<T> line, float grid_size)
        {
        }

        iterator begin() const {}
        iterator end() const {}

    private:

    };
}